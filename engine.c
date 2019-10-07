#include <limits.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <cairo-xcb.h>
#include <cairo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <xcb/xcb.h>

/* Our header files */
#include "audio.h"
#include "helpers.h"

void
draw(cairo_surface_t *backbuffer_surface,
     uint16_t width,
     uint16_t height) {

  static uint32_t offset = 0;

  /* Needed to ensure all pending draw operations are done */
  cairo_surface_flush(backbuffer_surface);

  int stride = cairo_image_surface_get_stride(backbuffer_surface) / 4;
  uint32_t *data = (uint32_t *)cairo_image_surface_get_data(backbuffer_surface);

  for(int i = 0; i < (stride*height); i++) {
    data[i] = setPixel(data[i], 0, 40, 82, 255);
  }

  for(int i = offset * stride; i < (stride*height); i++) {
    data[i] = setPixel(data[i], 255, 0, 0, 255);
  }

  /* Make sure that cached areas are re-read */ 
  /* Since we modified the pixel data directly without using cairo */
  cairo_surface_mark_dirty(backbuffer_surface);

  offset = (offset + 1) % height;
}

void
swapBuffers(cairo_t *front_cr,
            cairo_surface_t *backbuffer_surface) {

  cairo_set_source_surface(front_cr,
                           backbuffer_surface,
                           0,
                           0);

  cairo_paint(front_cr);
}

void
message_loop(xcb_connection_t *display,
             xcb_screen_t *screen,
             cairo_surface_t *frontbuffer_surface,
             cairo_t *front_cr) {

  struct timespec req = genSleep(0, 100000);
  struct timespec rem = genSleep(0, 0);

  xcb_configure_notify_event_t *configure_notify;
  xcb_expose_event_t *expose_event;
  xcb_key_press_event_t *key_event;

  int exposed = 0;
  int running = 1;

  uint16_t window_height = screen->height_in_pixels;
  uint16_t window_width = screen->width_in_pixels;

  cairo_surface_t *backbuffer_surface;

  while (running) {
    /* Poll for events */
    /* This is a non blocking function call */
    /* It will return NULL if there is no event on the queue */

    xcb_generic_event_t *event = xcb_poll_for_event(display);

    if (event != NULL) {
      switch (RECEIVE_EVENT(event)) {
        case XCB_KEY_PRESS:
          /* Quit on key press */
          key_event = (xcb_key_press_event_t *)event;
          printf("%u\n", key_event->detail);
          if (key_event->detail == 24) {
            running = 0;
          }
          break;
        case XCB_EXPOSE:
          printf("Got expose event\n");
          expose_event = (xcb_expose_event_t *)event;
          window_width = expose_event->width;
          window_height = expose_event->height;

          exposed = 1;
          break;

        case XCB_CONFIGURE_NOTIFY:
          configure_notify = (xcb_configure_notify_event_t *)event;

          if ((window_width != configure_notify->width) ||
              (window_height != configure_notify->height)) {
            /* Only redraw on configure_notify if the window dimensions have actually changed */

            printf("Got configure_notify event, w = %u, h = %u\n",
                   window_width,
                   window_height);
            window_height = configure_notify->height;
            window_width = configure_notify->width;
          }
          exposed = 1;
          break;
        default:
          break;
      }
      free(event);
    }

    if (exposed) {
      /* Create a new backbuffer to render to each time */
      backbuffer_surface = allocBackBuf(frontbuffer_surface, window_width, window_height);

      /* Flush any drawing operations to the front buffer */
      cairo_surface_flush(frontbuffer_surface);
      cairo_xcb_surface_set_size(frontbuffer_surface,
                                  window_width,
                                  window_height);

      draw(backbuffer_surface,
           window_width,
           window_height);

      /* This is where the magic happens */
      swapBuffers(front_cr,
                  backbuffer_surface);
      xcb_flush(display);

      /* Destroy the backbuffer surface every time */
      cairo_surface_destroy(backbuffer_surface);
    }

    nanosleep(&req, &rem);
  }
}

int
main(void) {
  srand(time(NULL));

  if (initializeSound() != 0) {
    /* FIXME should work without sound later */
    exit(1);
  }

  playFile("./sample.wav");

  /* Open up the display */
  xcb_connection_t *display = allocDisplay();

  /* Get a handle to the screen */
  xcb_screen_t *screen = allocScreen(display);

  int window_height = screen->height_in_pixels;
  int window_width = screen->width_in_pixels;

  /* Create a window */
  xcb_window_t window =
    allocWindow(display,
                screen,
                window_width,
                window_height);

  /* Map the window to the display */
  xcb_map_window(display, window);

  /* Allocate front buffer (X drawable) */
  cairo_surface_t *frontbuffer_surface =
    allocFrontBuf(display,
                  window,
                  screen,
                  window_width,
                  window_height);

  cairo_t *front_cr = cairo_create(frontbuffer_surface);

  message_loop(display,
               screen,
               frontbuffer_surface,
               front_cr);

  cairo_destroy(front_cr);
  cairo_surface_destroy(frontbuffer_surface);


  SDL_Quit();

  return 0;
}
