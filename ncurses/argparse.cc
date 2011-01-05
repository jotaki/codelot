/*
 * Filename: argparse.cc
 * Description: This is another simple application written to be used as an
 * example of programming an ncurses application around the C++ Wrapper.
 * This one adds the ability to parse command line arguments.
 * Compile With: g++ -o argparse argparse.cc -lncurses++ -lncurses
 * Example Usage: ./argparse What\'s Going On Folks?
 */

#include <cursesapp.h>
#include <string>

using std::string;

class ArgParseApp : public NCursesApplication
{
public:
	ArgParseApp() : NCursesApplication(FALSE)
	{
		// the default message.
		message = "Hello, World!";
	}

	void handleArgs(int argc, char *argv[]);
	int run();

private:
	string message;
};

void ArgParseApp::handleArgs(int argc, char *argv[])
{
	if(argc > 1)	// did we get args?
	{
		message = "";
		for(int i = 1; i < argc; ++i)
		{
			message = message + " " + argv[i];
		}
	}
}

int ArgParseApp::run()
{
	Root_Window->addstr(Root_Window->lines() / 2,
			(Root_Window->cols() - message.size()) / 2,
			message.c_str());

	Root_Window->refresh();

	Root_Window->getch();

	return 0;
}

static ArgParseApp argApp;
