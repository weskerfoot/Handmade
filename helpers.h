
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


