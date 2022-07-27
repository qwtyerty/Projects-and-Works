#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void childProc(string path, string args);
void runChild(char* job, char* inFolder, char* outFolder);
void rotate(vector<string> args, char* inFolder, char* outFolder);
void crop(vector<string> args, char* inFolder, char* outFolder);
void gray(vector<string> args, char* inFolder, char* outFolder);
void flip(vector<string> args, char* inFolder, char* outFolder);

/**
	Processes .job files from the commandline, job files contains lines of intsructions 
		to call files via exec to run prewritten excutible files which enact the specified 
		instruction onto a .tga image and uses fork for splitting processes 
**/

int main(int argc, char* argv[])
{
	/**
	 * Runs some input validation then sends the process to be processed by childProc
	 * @param argc: Number of arguments
	 * @param argv: String containing all the arguments
	 * @return returns 0 on success
	**/

	//Check for the right arguments in case something went wrong with bash
	if(argc != 4)
	{
		cout << "Usage <Path to .job file> <path to image folder> <path to output folder>" << endl;
		exit(1);
	}

	//Open the file
	ifstream inFile;
	inFile.open(argv[1]);

	//Check if the file even opened
	if(!inFile.is_open())
	{
		//Failed to open, print an error message
		cout << "Unable to open file '" << argv[1] << "' check log for more info" << endl;
		exit(2);
	}

	//Define a vector to hold the tasks and a temp string var
	vector<string> taskVector; 
	string temp;
	while(getline(inFile, temp))
	{
		//Read line and push_back into the vector
		if(temp[0] == 'e')
			break;
		taskVector.push_back(temp);
	}
	inFile.close();

	//Define an int to hold whether a process is a child or a parent
	int p;

	//Make sure that the image folder ends with a / so that it can be used to concatonate
	char* inFolder;
	int sizeInFolder = strlen(argv[2]);
	if(argv[2][sizeInFolder - 1] != '/')
	{
		inFolder = (char *) calloc(sizeInFolder + 1, sizeof(char));
		strcpy(inFolder, argv[2]);
		strcat(inFolder, "/");
	}
	else
		inFolder = argv[2];

	//Loop through forking for each process and then run the "child process"
	for(long unsigned int i = 0; i < taskVector.size(); ++i)

	{
		
		p = fork();
		if(p == 0)
		{
			//is the child
			runChild(const_cast<char*>(taskVector[i].c_str()), inFolder, argv[3]);
		}
		else if (p < 0)
			exit(2);
	}
	//Just wait 1000 muicroseconds to make sure the command line looks correct
	usleep(1000);
	return 0;
}

void runChild(char* job, char* inFolder, char* outFolder)
{
	/**
	 * Read a job, which is a single string, then pass it to a desegnated function
	 * @param job: String of a job to be performed
	 * @param inFolder: String containing the path to the folder where images are found
	 * @param outFolder: String containing the path to the folder where images will be output to
	 * **/

	//Set up a vector of stringa
	vector<string> args;

	//Split a given string into read segments using strtok
	char* readSeg;
	readSeg = strtok(job, " ");
	while(readSeg != NULL)
	{
		//Add the most recent chunk
		args.push_back(readSeg);
		//Read the next chunk (NULL tells strtok to read from the previous successful read)
		readSeg = strtok(NULL, " ");
	}

	//Send to a specific function based on the first character
	switch(args[0][0])
	{
		case 'r':
			//Send to rotate
			rotate(args, inFolder, outFolder);
			//In case of failure
			exit(10);
			break;

		case 'c':
			//Send to crop
			crop(args, inFolder, outFolder);
			//In case of failure
			exit(10);
			break;

		case 'g':
			//Send to gray
			gray(args, inFolder, outFolder);
			//In case of failure
			exit(10);
			break;
		
		case 'f':
			//Send to flip
			flip(args, inFolder, outFolder);
			//In case of failure
			exit(10);
			break;
		//Fun fact: I originally used a default, but it caused a psuedo fork bomb
	}
	return;
}

void crop(vector<string> args, char* inFolder, char* outFolder)
{
	/**
	 * Crops an image given specific dimensions
	 * @param args: Vecetor of string arguments that will be converted into an array of char*
	 * @param inFolder: String containing the path to the folder where images are found
	 * @param outFolder: String containing the path to the folder where images will be output to
	 * **/
	//Stores args[0] as a string
	string command = args[0]; 
	
	//Initialize a second iterator 
	int j = 1;
	//Allocate space in the outArgs
	char** outArgs = (char**) calloc(args.size() + 2, sizeof(char*)); 
	string temp;

	//Define outArgs[0]
	outArgs[0] = const_cast<char*>(("./" + command).c_str());
	
	//loop over the vector and assign each value of outArgs
	for(long unsigned int i = 1; i <= args.size(); ++i)
	{
		if(i == 1)
		{
			//Image path
			temp = inFolder + args[j];
			outArgs[i] = const_cast<char*>((temp).c_str());
		}
		else if(i == 2)
		{
			//Add output folder
			outArgs[i] = outFolder;
			--j;
		}
		else
			outArgs[i] = const_cast<char*>(args[j].c_str());
		//Increment j
		++j;
	}

	//Run the crop
	execvp(outArgs[0], outArgs);
	cout << outArgs[0] << " failed to exec, bailing" << endl;
	//Will hit an exit() call on return
	return;
}

void rotate(vector<string> args, char* inFolder, char* outFolder)
{
	/**
	 * Rotates an image given specific rotation
	 * @param args: Vecetor of string arguments that will be converted into an array of char*
	 * @param inFolder: String containing the path to the folder where images are found
	 * @param outFolder: String containing the path to the folder where images will be output to
	 * **/

	//Convert args[0] to a string
	string command = args[0];

	//Allocate memory for outArgs
	char** outArgs = (char**) calloc(args.size() + 2, sizeof(char*));
	string temp;
	
	//Loop over the vector and assign outArgs
	for(long unsigned int i = 1; i < args.size(); ++i)
	{
		//Check if .tga is in the argument
		if(args[i].find(".tga") != string::npos)
		{
			//Is a .tga file
			temp = inFolder + args[i];
			outArgs[i] = const_cast<char*>(temp.c_str());
		}
		else
			outArgs[i] = const_cast<char*>(args[i].c_str());
	}
	//Append outfolder and prepend command
	outArgs[args.size()] = outFolder;
	outArgs[0] = const_cast<char*>(("./" + command).c_str());
	execvp(outArgs[0], outArgs);
	cout << outArgs[0] << " failed to exec, bailing" << endl;
	//Will hit an exit() call on return
	return;
}


void gray(vector<string> args, char* inFolder, char* outFolder)
{
	/**
	 * Grays an image given
	 * @param args: Vecetor of string arguments that will be converted into an array of char*
	 * @param inFolder: String containing the path to the folder where images are found
	 * @param outFolder: String containing the path to the folder where images will be output to
	 * **/

	string command = args[0];
	
	//Allocate memory for outargs
	char** outArgs = (char**) calloc(args.size() + 2, sizeof(char*));
	string temp;

	//Loop over vector 
	for(long unsigned int i = 1; i < args.size(); ++i)
	{
		//Check if it is a .tga file
		if(args[i].find(".tga") != string::npos)
		{
			//Is a .tga image
			temp = inFolder + args[i];
			outArgs[i] = const_cast<char*>(temp.c_str());
		}
		else
			outArgs[i] = const_cast<char*>(args[i].c_str());
	}

	//Add in the two remaining arguments
	outArgs[args.size()] = outFolder;
	outArgs[0] = const_cast<char*>(("./" + args[0]).c_str());

	//Run gray
	execvp(outArgs[0], outArgs);
	cout << outArgs[0] << " failed to exec, bailing" << endl;
	return;
}

void flip(vector<string> args, char* inFolder, char* outFolder)
{
	/**
	 * Flips an image horizontally or vertically
	 * @param args: Vecetor of string arguments that will be converted into an array of char*
	 * @param inFolder: String containing the path to the folder where images are found
	 * @param outFolder: String containing the path to the folder where images will be output to
	 * **/

	
	string command = args[0]; 
	
	//Allocate memory for outArgs
	char** outArgs = (char**) calloc(args.size() + 2, sizeof(char*));
	string temp;

	//Loop over vector filling in outargs
	for(long unsigned int i = 1; i < args.size(); ++i)
	{
		//Check if argument is a .tga file
		if(args[i].find(".tga") != string::npos)
		{
			//Is a tga file
			temp = inFolder + args[i];
			outArgs[i] = const_cast<char*>(temp.c_str());
		}
		else
			outArgs[i] = const_cast<char*>(args[i].c_str());
	}

	//Fill the two remaining arguments
	outArgs[args.size()] = outFolder;
	outArgs[0] = const_cast<char*>(("./" + args[0]).c_str());

	//Run flip
	execvp(outArgs[0], outArgs);
	cout << outArgs[0] << " failed to exec, bailing" << endl;
	return;
}
