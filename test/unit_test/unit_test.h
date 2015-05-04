#ifndef __UNIT_TEST_H
#define __UNIT_TEST_H

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(msg, test) do { char *message = test(); \
                printf("running test: %s\n", msg); \
				if (message) return message; } while (0)
#endif /* __UNIT_TEST_H */
