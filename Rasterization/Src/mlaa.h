#ifndef MORPHOLOGICAL_ANTIALIASING
#define MORPHOLOGICAL_ANTIALIASING

#include <vector>

namespace CGCore
{

class MLAntialias
{
public:
	MLAntialias(unsigned char *framebuffer, size_t w, size_t h);
	

	void resolve();

private:
	struct Edge
	{
		int begin;
		int end;
		int pri;
		bool isEdgeBegin;
		bool isEdgeEnd;
		Edge(int b, int e, int p, bool iseb, bool ised) :
			begin(b), end(e), pri(p), isEdgeBegin(iseb), isEdgeEnd(ised)
		{
		}
	};
	void findAPrimaryEdges();
	void antialiasRowEdge(int xBegin, int xEnd, int yPri, bool isEdgeBegin, bool isEdgeEnd);
	void antialiasColEdge(int yBegin, int yEnd, int xPri, bool isEdgeBegin, bool isEdgeEnd);

	unsigned char*	  m_framebuffer;
	size_t			  m_height;
	size_t			  m_width;
	std::vector<Edge>  m_rowEdges;
	std::vector<Edge>  m_colEdges;
};

}

#endif