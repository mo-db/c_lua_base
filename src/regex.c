#include "regex.h"

bool regex_match(char* pattern, char* subject) {
	// --- variable definitions ---
	pcre2_code *re;
	PCRE2_SPTR pcre2_pattern;
	PCRE2_SPTR pcre2_subject;
	PCRE2_SPTR name_table;

	int errornumber;
	int find_all = 1;
	int caseless_match = 1;
	int i;
	int rc;

	uint32_t namecount;
	uint32_t name_entry_size;

	PCRE2_SIZE erroroffset;
	PCRE2_SIZE *ovector;
	PCRE2_SIZE ovector_last[2];
	PCRE2_SIZE subject_length;

	pcre2_match_data *match_data;

	pcre2_pattern = (PCRE2_SPTR)pattern;
	pcre2_subject = (PCRE2_SPTR)subject;
	subject_length = (PCRE2_SIZE)strlen(subject);

	// --- base ---
	re = pcre2_compile(
			pcre2_pattern,
			PCRE2_ZERO_TERMINATED,
			caseless_match,
			&errornumber,
			&erroroffset,
			NULL);

	/* Compilation failed: print the error message and exit. */
	if (re == NULL) {
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
		printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
			buffer);
		return 1;
	}

	match_data = pcre2_match_data_create_from_pattern(re, NULL);

	rc = pcre2_match(
			re,
			pcre2_subject,
			subject_length,
			0,
			0,
			match_data,
			NULL);

	/* Matching failed: handle error cases */
	if (rc < 0) {
		switch(rc)
			{
			case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
			/* Handle other cases if needed */
			default: printf("Matching error %d\n", rc); break;
			}
		pcre2_match_data_free(match_data);
		pcre2_code_free(re);
		return 1;
	}

	ovector = pcre2_get_ovector_pointer(match_data);
	printf("Match succeeded at offset %d\n", (int)ovector[0]);

	if (rc == 0)
		printf("ovector was not big enough for all the captured substrings\n");

	for (i = 0; i < rc; i++) {
		PCRE2_SPTR substring_start = pcre2_subject + ovector[2*i];
		PCRE2_SIZE substring_length = ovector[2*i+1] - ovector[2*i];
		printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
  }

	/**************************************************************************
	* That concludes the basic part of this demonstration program. We have    *
	* compiled a pattern, and performed a single match. The code that follows *
	* shows first how to access named substrings, and then how to code for    *
	* repeated matches on the same subject.                                   *
	**************************************************************************/

	/* See if there are any named substrings, and if so, show them by name. First
	we have to extract the count of named parentheses from the pattern. */

	(void)pcre2_pattern_info(
		re,                   /* the compiled pattern */
		PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
		&namecount);          /* where to put the answer */

	if (namecount == 0)
		printf("No named substrings\n");
	else
		{
		PCRE2_SPTR tabptr;
		printf("Named substrings\n");

		/* Before we can access the substrings, we must extract the table for
		translating names to numbers, and the size of each entry in the table. */

		(void)pcre2_pattern_info(
			re,                       /* the compiled pattern */
			PCRE2_INFO_NAMETABLE,     /* address of the table */
			&name_table);             /* where to put the answer */

		(void)pcre2_pattern_info(
			re,                       /* the compiled pattern */
			PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
			&name_entry_size);        /* where to put the answer */

		/* Now we can scan the table and, for each entry, print the number, the name,
		and the substring itself. In the 8-bit library the number is held in two
		bytes, most significant first. */

		tabptr = name_table;
		for (i = 0; i < namecount; i++)
			{
			int n = (tabptr[0] << 8) | tabptr[1];
			printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
				(int)(ovector[2*n+1] - ovector[2*n]), pcre2_subject + ovector[2*n]);
			tabptr += name_entry_size;
			}
		}


	/*************************************************************************
	* If the "-g" option was given on the command line, we want to continue  *
	* to search for additional matches in the subject string, in a similar   *
	* way to the /g option in Perl. This turns out to be trickier than you   *
	* might think because of the possibility of matching an empty string.    *
	*                                                                        *
	* To help with this task, PCRE2 provides the pcre2_next_match() helper.  *
	*************************************************************************/

	if (!find_all)     /* Check for -g */
		{
		pcre2_match_data_free(match_data);  /* Release the memory that was used */
		pcre2_code_free(re);                /* for the match data and the pattern. */
		return 0;                           /* Exit the program. */
		}

	/* Loop for second and subsequent matches */

	ovector_last[0] = ovector[0];
	ovector_last[1] = ovector[1];

	for (;;) {
		PCRE2_SIZE start_offset;
		uint32_t options;

		/* After each successful match, we use pcre2_next_match() to obtain the match
		parameters for subsequent match attempts. */

		if (!pcre2_next_match(match_data, &start_offset, &options))
			break;

		/* Run the next matching operation */

		rc = pcre2_match(
			re,                   /* the compiled pattern */
			pcre2_subject,              /* the subject string */
			subject_length,       /* the length of the subject */
			start_offset,         /* starting offset in the subject */
			options,              /* options */
			match_data,           /* block for storing the result */
			NULL);                /* use default match context */

		/* If this match attempt fails, exit the loop for subsequent matches. */

		if (rc == PCRE2_ERROR_NOMATCH)
			break;

		/* Other matching errors are not recoverable. */

		if (rc < 0)
			{
			printf("Matching error %d\n", rc);
			pcre2_match_data_free(match_data);
			pcre2_code_free(re);
			return 1;
			}

		/* This demonstration program depends on pcre2_next_match() to ensure that the
		loop for second and subsequent matches does not run forever. However, it would
		be robust practice for a production application to verify this. The following
		block of code shows how to do this. This error case is not reachable unless
		there is a bug in PCRE2.

		Because this program does not set the PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK option,
		the logic is simple. We verify that either ovector[1] has advanced, or that we
		have an empty match touching the end of a previous non-empty match. See the
		API documentation for guidance if your application uses
		PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK and searches for multiple matches. */

		if (!(ovector[1] > ovector_last[1] ||
					(ovector[1] == ovector[0] && ovector_last[1] > ovector_last[0] &&
					 ovector[1] == ovector_last[1])))
			{
			printf("\\K was used in an assertion to yield non-advancing matches.\n");
			printf("Run abandoned\n");
			pcre2_match_data_free(match_data);
			pcre2_code_free(re);
			return 1;
			}

		ovector_last[0] = ovector[0];
		ovector_last[1] = ovector[1];

		/* Match succeeded. */

		printf("\nMatch succeeded again at offset %d\n", (int)ovector[0]);

		/* The match succeeded, but the output vector wasn't big enough. This
		should not happen. */

		if (rc == 0)
			printf("ovector was not big enough for all the captured substrings\n");

		/* We guard against patterns such as /(?=.\K)/ that use \K in an assertion to
		set the start of a match later than its end. As explained above, this case
		should not occur because this demonstration program does not set the
		PCRE2_EXTRA_ALLOW_LOOKAROUND_BSK option, however, we do include code showing
		how to detect it. */

		if (ovector[0] > ovector[1])
			{
			printf("\\K was used in an assertion to set the match start after its end.\n"
				"From end to start the match was: %.*s\n", (int)(ovector[0] - ovector[1]),
					(char *)(pcre2_subject + ovector[1]));
			printf("Run abandoned\n");
			pcre2_match_data_free(match_data);
			pcre2_code_free(re);
			return 1;
			}

		/* As before, show substrings stored in the output vector by number, and then
		also any named substrings. */

		for (i = 0; i < rc; i++)
			{
			PCRE2_SPTR substring_start = pcre2_subject + ovector[2*i];
			size_t substring_length = ovector[2*i+1] - ovector[2*i];
			printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
			}

		if (namecount == 0)
			printf("No named substrings\n");
		else {
			PCRE2_SPTR tabptr = name_table;
			printf("Named substrings\n");
			for (i = 0; i < namecount; i++) {
				int n = (tabptr[0] << 8) | tabptr[1];
				printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
					(int)(ovector[2*n+1] - ovector[2*n]), pcre2_subject + ovector[2*n]);
				tabptr += name_entry_size;
			}
		}
	}      /* End of loop to find second and subsequent matches */

	printf("\n");

	pcre2_match_data_free(match_data);
	pcre2_code_free(re);
	return 0;

	/* End of pcre2demo.c */
}


#define STACK_MAX 255
typedef struct Stack {
	float values[STACK_MAX];
	int depth;
} Stack;

void push(Stack* stack, float f) {
}
float pop(Stack* stack) {
}


// must consist of at least two numbers
bool parse_postfix(char* str, float* result) {
	float stack[2];
	int stack_index = 0;

	// push until operator
	while(*str != '\0') {
		if (isdigit(*str)) {
			// handle single 0
			if (*str == 0 && !isdigit(*(str+1))) {
				stack[stack_index++] = *str;
				str++;
			} else {
				// parse float
				char* end_str = NULL;
				float temp = strtof(str, &end_str);
				if (temp == 0 || temp == HUGE_VALF) {
					return false;
				}
				stack[stack_index++] = temp;
				str = end_str;
			}
		} else {
			char c = *str;
			switch (c) {
				case '+':

			}
			//
			// if (
			// 	c == '+' ||
			// 	c == '-' ||
			// 	c == '*' ||
			// 	c == '/' ||
			// 	c == '%' ||
			// 	c == '>' ||
			// 	c == '<' ||
			// 	c == '=') {
			// 	if (c 
			// }
		}
	}
}
