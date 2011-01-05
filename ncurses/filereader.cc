/* 
 * Filename: filereader.cc
 * Description: In this example, we are gonna work with Panels / Frames
 * The idea here, is to make a simple file viewer. (Use ^X (Ctrl-X) to
 * quit this program)
 * Compile With: g++ -o filereader filereader.cc -lncurses++ -lncurses -lpanel
 */

#ifndef NULL
# define NULL 0
#endif

#include <cursesapp.h>
#include <cursesp.h>
#include <cursesf.h>
#include <cursesm.h>

#include <string>
#include <sstream>
#include <fstream>

using std::stringstream;
using std::string;
using std::ifstream;

class FileReaderApp : public NCursesApplication
{
public:
	FileReaderApp() : NCursesApplication(FALSE)
	{
		contents << "You didn't choose a file to read from.. *sigh*\n" <<
				"Well.. Just Press ^X (Ctrl-X) to exit this program.";
		maxcols = Root_Window->cols() + 1;
		maxlines = Root_Window->lines() + 1;
	}

	void handleArgs(int argc, char *argv[]);
	int run();

private:
	stringstream contents;
	int maxcols;
	int maxlines;
};

void FileReaderApp::handleArgs(int argc, char *argv[])
{
	ifstream file;
	string line;

	if(argc == 1)
		return;

	file.open(argv[1]);
	if(file.is_open())
	{
		contents.str("");
		maxlines = 0;
		maxcols = Root_Window->cols() + 1;

		while(!file.eof())
		{
			getline(file, line);

			if(line.length() > maxcols)
				maxcols = line.length();

			contents << line << "\n";
			++maxlines;
		}

		file.close();
	}
}

int FileReaderApp::run()
{
	NCursesPanel scrollPanel(Root_Window->lines(), Root_Window->cols(), 0, 0);
	NCursesFramedPad framePad(scrollPanel, maxlines + 1, maxcols + 12);
	int ch = 0;

	while((ch = contents.get()) != EOF)
	{
		framePad.addch(ch);
	}

	framePad();
	return 0;
}

static FileReaderApp readApp;
