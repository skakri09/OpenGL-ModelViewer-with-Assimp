#include "DirectoryBrowser.h"


DirectoryBrowser::DirectoryBrowser()
{
	showDirBrowser = false;
}

DirectoryBrowser::~DirectoryBrowser()
{
}


void DirectoryBrowser::RenderDirectoryBrowser(float mouseX, float mouseY, LeftMouseState* state)
{
	if(*state == FIRST_UP && previousState == FIRST_UP)
		*state = UP;
	else if(*state == FIRST_DOWN && previousState == FIRST_DOWN)
		*state = DOWN;
	previousState = *state;

	if(showDirBrowser)
	{
		if(DirStack.size() > 0)
		{
			std::vector<DirectoryEntry> dir = DirStack.top();
			for(unsigned int i=0; i < dir.size(); i++)
			{
				RenderEntry(&dir.at(i), mouseX, mouseY, state);
			}
			RenderEntry(&PopBack, mouseX, mouseY, state);
		}
	}

	//	if(std::regex_match(input, std::regex("(exit)")))
}

void DirectoryBrowser::RenderEntry(DirectoryEntry* d, float mouseX, float mouseY, LeftMouseState* state )
{
	Text t = d->text;
	if(t.Contains(GetNormMCoords(mouseX, mouseY)))
	{
		if(*state == FIRST_UP)
		{
			t.color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
			HandleCLickedEntry(*d);
		}
		else if(*state == FIRST_DOWN || *state == DOWN)
		{
			t.color = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
		}
		else if(*state == UP)
		{
			t.color = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
		}
	}
	else
		t.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	textRenderer->RenderText(t);
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
{
	this->gameManager = gameManager;
	textRenderer = std::make_shared<TextRenderer>();
	textRenderer->InitTextRenderer(textProgram);
	textPosition = glm::vec2(-1.0f, 0.9f);
	PaddingY = 2.0f/WH();
	previousState = UP;
	
	PopBack.text = textRenderer->GenerateText("..//", "calibri", textPosition.x, textPosition.y, ScaleX(0.8f), ScaleY(0.8f), glm::vec4(1));
	PopBack.extension = "pop";
	
	PushNewDirectory(dirPath);
	
}

void DirectoryBrowser::PushNewDirectory( path newDir )
{
	std::vector<DirectoryEntry> v;
	directory_iterator end;

	try	{
		if(exists(newDir)){
			float yOff = PopBack.text.h + PaddingY; 
			for(directory_iterator iter(newDir); iter != end; ++iter){
				path p = *iter;
				if(v.size()>0)
					yOff += v.front().text.h + PaddingY;
				if(is_regular_file(*iter)){
					DirectoryEntry t;
					t.file_size = file_size(*iter);
					t.extension = p.extension().string();
					std::stringstream str;
					str << p.string() << "   " << t.file_size << "KB";
					t.text = textRenderer->GenerateText(str.str(), "calibri", -1.0f, 0.9f-yOff, ScaleX(0.8f), ScaleY(0.8f), glm::vec4(1));
					t.p = p;
					v.push_back(t);
				}
				else if(is_directory(*iter))
				{
					DirectoryEntry t;
					t.extension = "folder";
					t.text = textRenderer->GenerateText(p.string()+"\\", "calibri", -1.0f, 0.9f-yOff, ScaleX(0.8f), ScaleY(0.8f), glm::vec4(1));
					t.p = p;
					v.push_back(t);
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
	if(DirStack.size() > 1)
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

float DirectoryBrowser::ScaleX(float scale)
{
	return scale/WW();
}

float DirectoryBrowser::ScaleY(float scale)
{
	return scale/WH();
}

glm::vec2 DirectoryBrowser::GetNormMCoords(float mouseX, float mouseY)
{
	float x = ((mouseX / static_cast<float>(WW()))*2.0f) - 1.0f;
	float y = 1.0f - ((static_cast<float>(mouseY) / WH()) * 2.0f);

	glm::vec2 coord = glm::vec2(x, y);

	return coord;
}

void DirectoryBrowser::HandleCLickedEntry( DirectoryEntry& entry )
{
	if(aiIsExtensionSupported(entry.extension.c_str()))
	{
		gameManager->LoadModel(entry.p.string());
	}
	else if(entry.extension == "folder")
	{
		PushNewDirectory(entry.p);
	}
	else if(entry.extension == "pop")
	{
		PopDirectory();
	}
}
