/*
 * Filename: helloworld.cc
 * Description: helloworld program written in ncurses using the C++ wrapper
 * Compile With: g++ -o helloworld helloworld.cc -lncurses++ -lncurses
 */

#include <cursesapp.h>
#include <string>

using std::string;

class HelloWorldApp : public NCursesApplication
{
public:
	// NCursesApplication(bool bColors) ie: use colors?
	HelloWorldApp() : NCursesApplication(FALSE) {}

	int run();	// This is basically the main() (no args are passed here)
};

int HelloWorldApp::run()
{
	string hello = "Hello, World!";

	// Root_Window is basically stdscr.
	Root_Window->addstr(Root_Window->lines() / 2,
			(Root_Window->cols() - hello.size()) / 2,
			hello.c_str());

	// refresh the screen.
	Root_Window->refresh();

	// Wait for user to press a key.
	Root_Window->getch();

	return 0;	// the return code.
}

// We need this so that the library will call our application.
static HelloWorldApp helloApp;
