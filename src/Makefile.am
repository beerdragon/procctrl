bin_PROGRAMS = procctrl
procctrl_SOURCES =	kill.c \
			main.c \
			params.c \
			process.c \
			query.c \
			start.c \
			stop.c \
			watchdog.c
check_PROGRAMS = unittest
unittest_SOURCES =	test_units.c \
			kill.c test_kill.c \
			params.c test_params.c \
			process.c test_process.c \
			query.c test_query.c \
			start.c test_start.c \
			stop.c test_stop.c \
			watchdog.c test_watchdog.c
unittest_LDADD = @CUNIT_LDFLAGS@
