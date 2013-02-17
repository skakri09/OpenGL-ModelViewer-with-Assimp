#include "DirectoryBrowser.h"


DirectoryBrowser::DirectoryBrowser()
{
	showDirBrowser = false;
}

DirectoryBrowser::~DirectoryBrowser()
{
}


void DirectoryBrowser::RenderDirectoryBrowser()
{
	if(showDirBrowser)
	{
		if(DirStack.size() > 0)
		{
			std::vector<Text> dir = DirStack.top();
			for(int i=0; i < dir.size(); i++)
				textRenderer->RenderText(dir.at(i));
		}
	}
}


void DirectoryBrowser::DisplayDirContent()
{
	std::cout << "Directory content: " << std::endl;
	directory_iterator end;

	//Wrapping the displaying of content in a try block, just in case
	//some bad person were to have deleted our directory while we were 
	//here playing with the model viewer
	try
	{
		if(exists(curDir))
		{
			//Looping trough the contents of the current directory
			for(directory_iterator iter(curDir); iter != end; ++iter)
			{
				path p = *iter;
				//If what the iterator points to is a regular file, it's name 
				//and file size is written to console
				if(is_regular_file(*iter))
				{
					float size = static_cast<float>(file_size(*iter));
					size /= 1000.0f;
					if(size > 1000.0f)
					{
						size /= 1000;
						std::cout << p.string() << "\t\t" << size << "MB" << std::endl;
					}
					else
						std::cout << p.string() << "\t\t" << size << "KB" << std::endl;
				}
				//If the iterator points to a sub-directory, the folder name is written to console
				else if(is_directory(*iter))
					std::cout << p.string() << "\\" << "\t\t" << "folder" << std::endl;
			}
		}
	}
	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void DirectoryBrowser::Init( std::string dirPath, GameManager* gameManager, 
							std::shared_ptr<GLUtils::Program> textProgram)
							//unsigned int windowWidth, unsigned int windowHeight )
{
	this->gameManager = gameManager;
	textRenderer = std::make_shared<TextRenderer>();
	textRenderer->InitTextRenderer(textProgram);

	PaddingY = 2.0f/WH();
	PushNewDirectory(dirPath);
}

void DirectoryBrowser::EnterConsoleMode()
{
	//std::cout << "\n\nEntered console-mode. Enter a command, or type 'help'." << std::endl;
	//bool consoleMode = true;

	////Processing input from the console while consoleMode is true.
	////This will be the case until exit is typed by user, or a model is loaded.
	//while(consoleMode)
	//{
	//	std::cout << "\\>";
	//	std::string input;
	//	std::getline(std::cin, input);
	//	if(std::regex_match(input, std::regex("(exit)")))
	//	{
	//		consoleMode = false;
	//		break;
	//	}
	//	consoleMode = !ProcessInput(input);
	//	std::cout << std::endl;
	//}
	//std::cout << "\n\nExited console-mode. You may resume using the model viewer." << std::endl;
}

void DirectoryBrowser::PushNewDirectory( path newDir )
{
	/*testText = textRenderer->GenerateText("test text", "calibri", -1, 0, 2.0f/gameManager->WindowWidth(),
	2.0f/gameManager->WindowHeight(), glm::vec4(1.0f));*/
	std::vector<Text> v;
	directory_iterator end;

	try	{
		if(exists(newDir)){
			float yOff = 0.0f;
			for(directory_iterator iter(newDir); iter != end; ++iter){
				path p = *iter;
				
				if(is_regular_file(*iter)){
					Text t;
					float size = static_cast<float>(file_size(*iter));
					
					if(v.size()>0)
						yOff += v.front().h + PaddingY;
					t = textRenderer->GenerateText(p.string(), "calibri", -1.0f, 0.9f-yOff, 0.5f/WW(), 0.5f/WH(), glm::vec4(1));
					v.push_back(t);
				}
				else if(is_directory(*iter))
				{

				}

			}
		}
		DirStack.push(v);
	}
	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	DirStack.push(v);
}

void DirectoryBrowser::PopDirectory()
{
	if(DirStack.size() > 0)
		DirStack.pop();
}

unsigned int DirectoryBrowser::WW()
{
	return gameManager->WindowWidth();
}

unsigned int DirectoryBrowser::WH()
{
	return gameManager->WindowHeight();
}
