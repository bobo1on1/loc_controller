#ifndef DEBUGPRINT_H
#define DEBUGPRINT_H

#define DEBUG 0

#if DEBUG
#define DBGPRINT(str, ...) printf_P(PSTR("%i %lu: " str), __LINE__, millis(), ##__VA_ARGS__)
#define DBGPRINTAUX(str, ...) printf_P(PSTR(str), ##__VA_ARGS__)
#else
#define DBGPRINT(x, ...)
#define DBGPRINTAUX(x, ...)
#endif

#endif //DEBUGPRINT_H
