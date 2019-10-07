/* Allocation functions */

static xcb_visualtype_t*
findVisual(xcb_connection_t *display,
            xcb_visualid_t visual) {

  /* Taken from here https://cairographics.org/cookbook/xcbsurface.c/ */
  /* This function basically searches for a xcb_visualtype_t given a visual ID */

  xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(display));

  for(; screen_iter.rem; xcb_screen_next(&screen_iter)) {
    /* Iterate over the screens available */

    xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen_iter.data);

    for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
      /* Iterate over the depths allowed on this screen */
      xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
      /* depth_iter.data = the number of visuals available */

      for (; visual_iter.rem; xcb_visualtype_next(&visual_iter)) {
        /* Iterate over all of the visuals available */
        if (visual == visual_iter.data->visual_id) {
          printf("%u bits per rgb value\n", visual_iter.data->bits_per_rgb_value);
          return visual_iter.data;
        }
      }
    }
  }
  return NULL;
}



xcb_screen_t*
allocScreen(xcb_connection_t *display) {
  /* Gets a screen from the display connection */
  const xcb_setup_t *setup = xcb_get_setup(display);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  xcb_screen_t *screen = iter.data;

  return screen;
}

cairo_surface_t*
allocFrontBuf(xcb_connection_t *display,
              xcb_drawable_t drawable,
              xcb_screen_t *screen,
              int width,
              int height) {

  cairo_surface_t *surface = cairo_xcb_surface_create(display,
                                                      drawable,
                                                      findVisual(display, screen->root_visual),
                                                      width,
                                                      height);

  printf("Stride = %d\n", cairo_image_surface_get_stride(surface));

  return surface;
}

cairo_surface_t*
allocBackBuf(cairo_surface_t *frontBuf,
             int width,
             int height) {

  cairo_format_t format = CAIRO_FORMAT_ARGB32;

  cairo_surface_t *surface = cairo_surface_create_similar_image(frontBuf,
                                                                format,
                                                                width,
                                                                height);

  return surface;
}

xcb_connection_t*
allocDisplay() {
  /* Get a display to use */
  /* Currently just uses the default display */

  xcb_connection_t *display = xcb_connect(NULL, NULL);

  if (display == NULL) {
    fprintf(stderr, "Could not open the display! :(\n");
    exit(1);
  }
  return display;
}

xcb_window_t
allocWindow(xcb_connection_t *display,
            xcb_screen_t *screen,
            uint16_t width,
            uint16_t height) {
  /* Create the window */
  xcb_window_t window = xcb_generate_id(display);

  xcb_cw_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  /* Define all the events we want to handle with xcb */
  /* XCB_EVENT_MASK_EXPOSURE is the "exposure" event */
  /* I.e. it fires when our window shows up on the screen */

  uint32_t eventmask = (XCB_EVENT_MASK_EXPOSURE |
                        XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                        XCB_EVENT_MASK_KEY_PRESS);

  /* Passed to the window to mask on events */
  /* a single white pixel is allocated */
  /* Actual rendering is done directly, and not using X rendering functions */
  uint32_t valwin[2] = {screen->white_pixel, eventmask};

  xcb_create_window(display,
                    XCB_COPY_FROM_PARENT,  /* depth (same as root) */
                    window,
                    screen->root, /* parent window */
                    0, /* x */
                    0, /* y */
                    width,/* width */
                    height,/* height */
                    10, /* border_width  */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class */
                    screen->root_visual, /* visual */
                    mask, /* value mask, used for events */
                    valwin); /* masks, used for events */

  return window;
}

/* Misc helpers */

static struct timespec
genSleep(time_t sec,
         long nanosec) {
  struct timespec t;
  t.tv_sec = sec;
  t.tv_nsec = nanosec;
  return t;
}

void
printCairoFormat(cairo_format_t format) {
  switch (format) {
    case CAIRO_FORMAT_INVALID:
      printf("Invalid\n");
      break;
    case CAIRO_FORMAT_ARGB32:
      printf("ARGB32\n");
      break;
    case CAIRO_FORMAT_RGB24:
      printf("ARGB32\n");
      break;
    case CAIRO_FORMAT_A8:
      printf("A8\n");
      break;
    case CAIRO_FORMAT_A1:
      printf("A1\n");
      break;
    case CAIRO_FORMAT_RGB16_565:
      printf("RGB16_565\n");
      break;
    case CAIRO_FORMAT_RGB30:
      printf("RGB30\n");
      break;
    default:
      break;
  }
}

/* Macro definition to parse X server events
 * The ~0x80 is needed to get the lower 7 bits
 * It basically sets the first 7 bits to 1, and the last to 0, then ands them
 * which gives you the lower 7 bits!
 * Don't ask me why they felt the need to ignore the last bit
 */
#define RECEIVE_EVENT(ev) (ev->response_type & ~0x80)

/* Macros for setting and getting pixels */
/* Implements premultiplied alpha channels */
#define A(p) ((p) & 0xff000000)
#define R(p) ((p) & 0x00ff0000)
#define G(p) ((p) & 0x0000ff00)
#define B(p) ((p) & 0x000000ff)
#define setA(p, v) ((p) | ((v) << 24))
#define setR(p, v, a) ((p) | ((uint32_t)((a) * ((v) << 16))) )
#define setG(p, v, a) ((p) | ((uint32_t)((a) * ((v) << 8))) )
#define setB(p, v, a) ((p) | ((uint32_t)((a) * (v))) )

static uint32_t
setPixel(uint32_t p,
         uint8_t r,
         uint8_t g,
         uint8_t b,
         uint8_t a) {
  /* Set ARGB pixel */
  /* Use premultiplied alpha channels */
  float af = a / 255.0;
  return setB(setG(setR(setA(p, a), r, af), g, af), b, af);
}
