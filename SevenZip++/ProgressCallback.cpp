#include "ProgressCallback.h"

CProgressCallback::CProgressCallback(std::function<void(int step, int maxSteps)> callback)
{
	m_callback = callback;
	m_size = 0;
	m_processed = 0;
}

CProgressCallback::~CProgressCallback()
{
}

/* void CProgressCallback::addPath(std::string path, int size) {
	CPath *p = new CPath;
	p->path = path;
	p->size = size;
	m_size += size;
	m_paths.insert(m_paths.end(), p);
} */

void CProgressCallback::setSize(int size) {
	m_size = size;
}

void CProgressCallback::processed(int processed) {
	m_processed += processed;
	// send update
	if (m_callback && processed > 0) {
		m_callback(m_processed, m_size);
	}
}


/*void CProgressCallback::setFinished(std::string path) {
	for (int i = 0; i < m_paths.size(); i++) {
		CPath *cur = m_paths.at(i);
		if (cur->path.compare(path) == 0) {
			m_processed += cur->size;
			if (m_callback && cur->size > 0) {
				m_callback(m_processed, m_size);
			}
			m_paths.erase(m_paths.begin() + i);
			break;
		}
	}
} */