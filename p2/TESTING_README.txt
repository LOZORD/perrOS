To run the tests, execute
python ~cs537-1/testing/p2a/ShellTest.pyc <path-to-your-shell-test-folder>



The tests descriptions are below:-

1. Build	-	Builds the code by calling make. If the Makefile is correct, and the build is successful, this test will pass

2. Exit		-	Calls 'exit' to quit the shell. If this test fails, the remaining tests will not be run

3. Basic	-	Calls 'ls' (or a similar command) and then 'exit'. Tests if the command's output is correct.

4. Blankline	-	Simply hits enter (CRLF) and sees if the prompt comes back and the shell is still running, and then calls exit.

5. Multiline	-	Tests multiple commands issued one after another, and tests if all of them work. 

6. ExitError	-	Calls 'exit sometexthere' and this shouldn't quit the shell, but raise an error.

7. BadCommand	-	Tests an invalid command and tests if the shell raises an error.

8. CurDir	-	Tests the functionality of 'pwd'

9. ChDirBasic	-	Tests the functionality of 'cd' with a valid directory

10. ChDirError	-	Tests if the shell raises an error when 'cd' is executed with an invalid directory.

11. ParseArgsOne	-	Tests if an executable accepts one command line argument correctly (e.g. ls -l)

12. ParseArgsTwo	-	Tests if an executable accepts two command line arguments correctly (e.g. uname -r -v)

13. WhiteSpaceOne	-	Tests preceeding white spaces on commands. (e.g. mysh>		uname)

14. WhiteSpaceTwo	-	Tests white spaces between arguments to commands (e.g. mysh>uname	-r)

15. LongLine		-	Tests a very long command (with around 900 characters)

16. RedirectOne		-	Tests simple redirection without arguments (e.g. mysh>ls > outfile)

17. RedirectTwo		-	Tests append redirection without arguments (e.g. mysh>ls >> outfile)

18. PipeOne		-	Tests basic pipe functionality (e.g. ls | wc)

19. PipeTwo		-	Tests pipe functionality with 1 argument (e.g. ls -l | wc)

20. TestTee		-	Tests the tee functionality by doing echo hello % cat and vrifyinf the tee.txt output

21. MultiplePipes	-	Test multiple pipes (e.g. echo Hello how are you | grep Hello | wc)

22. PipeRedirect	-	Tests pipe and redirect together (e.g. uname | wc > out)

23. ParseArgsThree	-	Parses three arguments like "echo Hi Hello! How are you?"

24. RedirectThree	-	Tests append redirection on an existing file like echo hello >> existing_file

25. RedirectFour	-	Tests basic redirection on an existing file like echo hello > existing_file

26. PipeThree	-	Tests pipe with arguments on both commands like echo Hi How are you? | grep -n Hi

27. PipeBadCommand	-	Tests pipe with a bad command like echo hello | badcommand
