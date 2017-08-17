#pragma once

#include <iostream>
#include <functional>
#include <vector>

struct CPath {
	std::string path;
	unsigned int size;
};

class CProgressCallback
{
	std::vector<CPath *> m_paths;
	std::function<void(int step, int maxSteps)> m_callback;
	int m_size;
	int m_processed;
public:
	
	CProgressCallback(std::function<void(int step, int maxSteps)> callback);
	//void addPath(std::string path, int size);
	//void setFinished(std::string path);
	void setSize(int size);
	void processed(int processed);
	int getSize() { return m_size; }
	~CProgressCallback();

};

