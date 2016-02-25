My code creates a socket and sets it to the parameters passed by the user.

It then reads from the socket and processes those requests accordingly.

It supports GET and everything else will give a bad request code.

If the file is not found or not able to be read or outside the root directory then it will give a not found code.

If the file is found and can be read the contents will be displayed and it will give a ok code.

It also logs these codes with the date, data read from the socket and the directory.

It will keep performing these actions until 'q' is pressed in which the program will gracefully exit.
