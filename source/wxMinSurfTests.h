/**
 wxMinSurfTests.h adapted from isosurf.h by Brian Paul and Wolfram Gloger
 */

#ifndef _WXMINSURFTESTS
#define _WXMINSURFTESTS 1
#define TRY_WX 1	//	compiles with wxWidgets window if TRY_WX is set to 1; this can be partially avoided by setting TRY_WX to 0
#if TRY_WX == 1
#if defined(__WXMSW__) || defined(__WINDOWS__)
#include <windows.h>
#endif
// we need OpenGL headers for GLfloat/GLint types used below
#if defined(__WXMAC__)
#   ifdef __DARWIN__
#       include <OpenGL/gl.h>
#       include <OpenGL/glu.h>
#   else
#       include <OpenGL/gl.h> // doh added directory
#       include <OpenGL/glu.h> // doh added directory
#   endif
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#endif
// the maximum number of vertex in the loaded .dat file
#define MAXVERTS     1000000
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif
#include "wx/glcanvas.h"
#include <map>
#include <vector>

extern unsigned int ccSelected, bbSelected, vertSelected;
extern bool segIsSelected;


class MyFrame;

// Define a new application type
class MyApp : public wxApp
{
public:
    virtual bool OnInit();// doh moved override: wxOVERRIDE;
	MyFrame *myFrame;

    //virtual void OnInitCmdLine(wxCmdLineParser& parser) wxOVERRIDE;
    //virtual bool OnCmdLineParsed(wxCmdLineParser& parser) wxOVERRIDE;
};

wxDECLARE_APP(MyApp);


// The OpenGL-enabled canvas
class TestGLCanvas : public wxGLCanvas
{
public:
    TestGLCanvas(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 int *gl_attrib = NULL);

    virtual ~TestGLCanvas();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnChar(wxKeyEvent& event);
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
    void OnMouseEvent(wxMouseEvent& event);

	//surface is ordered (x0, y0, z0, nx0, ny0, nz0, x1,..., nzi) for i triangles
	void newSurface(const std::vector<double> &surface);

	//scaffold is ordered (x0, y0, z0, r0, g0, b0, a0, x1,..., ai) for i vertices for GL_LINE_STRIP
	void newScaffold(const std::vector<double> &surface);

	//points is ordered (x0, y0, z0, r0, g0, b0, a0, x1,..., ai) for i vertices for GL_POINTS
	void newPoints(const std::vector<double> &points);

	//lines is ordered (x0, y0, z0, r0, g0, b0, a0, x1,..., ai) for i vertices for GL_POINTS
	void newLines(const std::vector<double> &lines);

	void addLine(const std::vector<double> &line);

	void updateCols(std::map<unsigned int, std::vector<double> > &uc);

	// vector in groups of 8: (x, y, z) (r, g, b, a) <number>
	void newNumbers(const std::vector<double> &numbers);

	void addNumber(double w, double x, double y, double z, double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0);

    //void LoadSurface(const wxString& filename);
    void InitMaterials();
    void InitGL();

private:
    wxGLContext* m_glRC;

    GLfloat m_verts[MAXVERTS][3];
    GLfloat m_norms[MAXVERTS][3];
	GLfloat m_cols[MAXVERTS][4];

	GLfloat m_numberverts[MAXVERTS][3];
	double m_numbers[MAXVERTS];
	GLfloat m_numbercols[MAXVERTS][4];

    GLint m_numverts, m_numlineverts, m_numnumbers;

	int winSize;

    GLfloat m_xrot;
    GLfloat m_yrot;

	GLfloat m_zoom, m_xtrans, m_ytrans, m_ztrans;
	GLfloat m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, m_upx, m_upy, m_upz, m_rightx, m_righty, m_rightz;

	//GLdouble rayx0, rayy0, rayz0, rayx1, rayy1, rayz1;
	GLdouble ray[6];

	GLfloat centerToEye[3]; // for normals

	GLfloat zNear, zFar;

	GLboolean modifierIsDown;

	void updateTranslation(double dx, double dy, double dz);
	void positionEye(double dx, double dy, double dz);
	void processClick(float x, float y);
	int getNewSize(int oldSize, int x, int y);

    wxDECLARE_NO_COPY_CLASS(TestGLCanvas);
    wxDECLARE_EVENT_TABLE();
};


// The frame containing the GL canvas
class MyFrame : public wxFrame
{
public:
    MyFrame(wxFrame *frame,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE);

    virtual ~MyFrame();

    TestGLCanvas *m_canvas;

	void updateStatus(std::string s);

private :
    void OnExit(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif

#endif // _WX_ISOSURF_H_