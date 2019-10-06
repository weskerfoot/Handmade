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
