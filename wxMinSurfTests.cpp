#include "wxMinSurfTests.h"

#if TRY_WX == 1

// adapted from isosurf.cpp by Brian Paul and Wolfram Gloger

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

#include "wx/timer.h"
#include "wx/glcanvas.h"
#include "wx/math.h"
#include "wx/log.h"
#include "wx/cmdline.h"
#include "wx/wfstream.h"
#include "wx/zstream.h"
#include "wx/txtstrm.h"

#include "kiss.h"
#include "minSurfTests.h"
#include "utilMinSurfTests.h"
#include "wxMinSurfTests.h"

#include <future>
#include <thread>

GLboolean g_use_vertex_arrays = GL_FALSE; // Mac requires false
GLboolean g_doubleBuffer = GL_TRUE;

std::future<void> fut, futSelected;

string selectedStatusString("");

unsigned int ccSelected(0), bbSelected(0), vertSelected(0), labelStatus(0);
bool segIsSelected(false), updateSelectedStatus(false);

GLclampf clearColor[] = {1.0, 1.0, 1.0, 0.0};
GLdouble centerColor[] = {0.0, 1.0, 0.0, 1.0},
	selectColor[] = {0.0, 0.0, 1.0, 1.0},
	highlightColor1[4] = {1.0, 0.5, 0.5, 1.0},
	highlightColor2[4] = {0.5, 1.0, 0.5, 1.0},
	highlightColor3[4] = {0.5, 0.5, 1.0, 1.0};

GLubyte bitZero16[] = {0, 0, 3, 192, 15, 240, 28, 56, 48, 12, 48, 12, 96, 6, 96, 6,
					96, 6, 96, 6, 48, 12, 48, 12, 28, 56, 15, 240, 3, 192, 0, 0},
		bitOne16[] = {0, 0, 0, 192, 0, 192, 0, 192, 0, 192, 0, 192, 0, 192, 0, 192,
					0, 192, 0, 192, 0, 192, 0, 192, 3, 192, 1, 192, 0, 192, 0, 0},
		bitTwo16[] = {0, 0, 127, 254, 127, 254, 224, 0, 60, 0, 31, 0, 7, 224, 1, 248,
					0, 60, 0, 28, 96, 6, 96, 6, 56, 28, 31, 248, 7, 224, 0, 0},
		bitThree16[] = {0, 0, 15, 248, 31, 252, 48, 14, 96, 6, 96, 6, 0, 28, 1, 248,
					1, 248, 0, 28, 96, 6, 96, 6, 48, 14, 31, 252, 15, 248, 0, 0},
		bitFour16[] = {0, 0, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 0, 24, 127, 254,
					127, 254, 96, 24, 96, 24, 96, 24, 96, 24, 96, 24, 96, 24, 0, 0},
		bitFive16[] = {0, 0, 31, 248, 63, 252, 48, 14, 96, 6, 96, 6, 0, 6, 0, 6,
					112, 62, 127, 248, 111, 224, 96, 0, 96, 0, 127, 254, 127, 254, 0, 0},
		bitSix16[] = {0, 0, 3, 240, 15, 248, 28, 28, 60, 6, 108, 6, 110, 6, 103, 252,
					99, 248, 96, 0, 48, 0, 48, 0, 28, 0, 15, 192, 3, 192, 0, 0},
		bitSeven16[] = {0, 0, 1, 192, 1, 192, 0, 192, 0, 224, 0, 96, 0, 112, 0, 112, 0, 48,
					0, 56, 0, 24, 0, 28, 0, 12, 0, 14, 127, 254, 127, 254, 0, 0},
		bitEight16[] = {0, 0, 7, 224, 31, 248, 56, 28, 96, 6, 96, 6, 56, 28, 31, 248,
					31, 248, 56, 28, 96, 6, 96, 6, 56, 28, 31, 248, 7, 224, 0, 0},
		bitNine16[] = {0, 0, 0, 6, 0, 6, 0, 6, 0, 6, 0, 6, 0, 6, 31, 254,
					63, 254, 112, 14, 96, 6, 96, 6, 112, 14, 63, 254, 31, 252, 0, 0};

GLubyte bitZero8[] = {0, 60, 102, 66, 66, 102, 60, 0},
		bitOne8[] = {0, 8, 8, 8, 8, 24, 8, 0},
		bitTwo8[] = {0, 126, 32, 28, 2, 66, 60, 0},
		bitThree8[] = { 0, 60, 66, 2, 12, 66, 60, 0},
		bitFour8[] = { 0, 4, 4, 4, 126, 68, 68, 0},
		bitFive8[] = { 0, 124, 2, 2, 124, 64, 126, 0},
		bitSix8[] = { 0, 60, 114, 94, 64, 64, 56, 0},
		bitSeven8[] = { 0, 16, 16, 8, 4, 2, 126, 0},
		bitEight8[] = {0, 60, 66, 66, 60, 66, 60, 0},
		bitNine8[] = {0, 2, 2, 62, 66, 66, 60, 0};

GLubyte bitSpace16f[64], bitPeriod16f[64], bitDash16f[64], bitZero16f[64], bitOne16f[64], bitTwo16f[64], bitThree16f[64], bitFour16f[64], bitFive16f[64], bitSix16f[64], bitSeven16f[64], bitEight16f[64], bitNine16f[64];
GLubyte bitSpace8f[32], bitPeriod8f[32], bitDash8f[32], bitZero8f[32], bitOne8f[32], bitTwo8f[32], bitThree8f[32], bitFour8f[32], bitFive8f[32], bitSix8f[32], bitSeven8f[32], bitEight8f[32], bitNine8f[32];

void initBitmapNumbers(){
	for(unsigned int i(0); i < 16; i++){
		for(unsigned int j(0); j < 2; j++){
			bitZero16f[4*i + j] = bitZero16[2*i + j];
			bitOne16f[4*i + j] = bitOne16[2*i + j];
			bitTwo16f[4*i + j] = bitTwo16[2*i + j];
			bitThree16f[4*i + j] = bitThree16[2*i + j];
			bitFour16f[4*i + j] = bitFour16[2*i + j];
			bitFive16f[4*i + j] = bitFive16[2*i + j];
			bitSix16f[4*i + j] = bitSix16[2*i + j];
			bitSeven16f[4*i + j] = bitSeven16[2*i + j];
			bitEight16f[4*i + j] = bitEight16[2*i + j];
			bitNine16f[4*i + j] = bitNine16[2*i + j];
		}
		bitZero16f[4*i + 2] = bitZero16f[4*i + 3] = 0;
		bitOne16f[4*i + 2] = bitOne16f[4*i + 3] = 0;
		bitTwo16f[4*i + 2] = bitTwo16f[4*i + 3] = 0;
		bitThree16f[4*i + 2] = bitThree16f[4*i + 3] = 0;
		bitFour16f[4*i + 2] = bitFour16f[4*i + 3] = 0;
		bitFive16f[4*i + 2] = bitFive16f[4*i + 3] = 0;
		bitSix16f[4*i + 2] = bitSix16f[4*i + 3] = 0;
		bitSeven16f[4*i + 2] = bitSeven16f[4*i + 3] = 0;
		bitEight16f[4*i + 2] = bitEight16f[4*i + 3] = 0;
		bitNine16f[4*i + 2] = bitNine16f[4*i + 3] = 0;
	}
	for(unsigned int i(0); i < 64; i++)
		bitSpace16f[i] = bitPeriod16f[i] = bitDash16f[i] = 0;
	bitPeriod16f[4] = bitPeriod16f[8] = bitPeriod16f[12]= 3;
	bitPeriod16f[5] = bitPeriod16f[9] = bitPeriod16f[13] = 192;
	bitDash16f[28] = bitDash16f[32] = 63;
	bitDash16f[29] = bitDash16f[33] = 252;

	for(unsigned int i(0); i < 8; i++){
		bitZero8f[4*i] = bitZero8[i];
		bitOne8f[4*i] = bitOne8[i];
		bitTwo8f[4*i] = bitTwo8[i];
		bitThree8f[4*i] = bitThree8[i];
		bitFour8f[4*i] = bitFour8[i];
		bitFive8f[4*i] = bitFive8[i];
		bitSix8f[4*i] = bitSix8[i];
		bitSeven8f[4*i] = bitSeven8[i];
		bitEight8f[4*i] = bitEight8[i];
		bitNine8f[4*i] = bitNine8[i];
		for(unsigned int j(1); j < 4; j++){
			bitZero8f[4*i + j] = 0;
			bitOne8f[4*i + j] = 0;
			bitTwo8f[4*i + j] = 0;
			bitThree8f[4*i + j] = 0;
			bitFour8f[4*i + j] = 0;
			bitFive8f[4*i + j] = 0;
			bitSix8f[4*i + j] = 0;
			bitSeven8f[4*i + j] = 0;
			bitEight8f[4*i + j] = 0;
			bitNine8f[4*i + j] = 0;
		}
	}
	for(unsigned int i(0); i < 16; i++)
		bitSpace8f[i/2] = bitPeriod8f[i] = bitDash8f[i] = 0;
	bitPeriod8f[2] = bitPeriod8f[4] = 24;
	bitDash8f[10] = 60;
}


//---------------------------------------------------------------------------
// MyApp
//---------------------------------------------------------------------------

wxIMPLEMENT_APP(MyApp);


bool MyApp::OnInit(){
    if ( !wxApp::OnInit() )
        return false;

	initBitmapNumbers();

#if TRY_WX==1


    // Create the main frame window
    myFrame = new MyFrame(NULL, wxT("Adapted from wxWidgets OpenGL Isosurf Sample.."), wxPoint(50, 50), wxSize(640, 710)); // taller for menu and status
   
	fut = std::async(sphereCoarsenTest);// wxTest sphereCoarsenTest

    return true;
#endif
	return false;
}

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
wxEND_EVENT_TABLE()

MyFrame::MyFrame(wxFrame *frame, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
		: wxFrame(frame, wxID_ANY, title, pos, size, style), m_canvas(NULL){

    wxMenu *fileMenu = new wxMenu; //menubar

    fileMenu->Append(wxID_EXIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

#ifdef __WXMSW__
    int *gl_attrib = NULL;
#else
    int gl_attrib[20] = { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1,
        WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 1,
        WX_GL_DOUBLEBUFFER,
#  if defined(__WXMAC__)  || defined(__WXQT__)
        GL_NONE };
#  else
        None };
#  endif
#endif

    if (!g_doubleBuffer){
        wxLogWarning("Disabling double buffering");
#ifdef __WXGTK__
        gl_attrib[9] = None;
#endif
        g_doubleBuffer = GL_FALSE;
    }

    m_canvas = new TestGLCanvas(this, wxID_ANY, gl_attrib);

    
    Show(true); // show frame
    Raise();

    m_canvas->InitGL();

	CreateStatusBar();
}

MyFrame::~MyFrame(){ delete m_canvas; }

// Intercept menu commands
void MyFrame::OnExit( wxCommandEvent& WXUNUSED(event) ){
    Close(true);// true is to force the frame to close
}

void MyFrame::updateStatus(std::string s){
	wxGetApp().myFrame->SetStatusText(wxString(s) + " " + selectedStatusString);
}


//---------------------------------------------------------------------------
// TestGLCanvas
//---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_SIZE(TestGLCanvas::OnSize)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_CHAR(TestGLCanvas::OnChar)
	EVT_KEY_DOWN(TestGLCanvas::OnKeyDown)
	EVT_KEY_UP(TestGLCanvas::OnKeyUp)
    EVT_MOUSE_EVENTS(TestGLCanvas::OnMouseEvent)
wxEND_EVENT_TABLE()

TestGLCanvas::TestGLCanvas(wxWindow *parent, wxWindowID id, int* gl_attrib)
		: wxGLCanvas(parent, id, gl_attrib){
    m_xrot = 0;
    m_yrot = 0;
	m_xtrans = 0;
	m_ytrans = 0;
	m_ztrans = 0.0f;
	m_zoom = -0.3f;

	m_eyex = 0.0f;
	m_eyey = 0.0f;
	m_eyez = 5.0f;
	m_centerx = m_centery = m_centerz = 0.0f;
	m_upx = 0.0;
	m_upy = 1.0;
	m_upz = 0.0;
	m_rightx = 1.0;
	m_righty = 0.0;
	m_rightz = 0.0;

	zNear = 5.0f;
	zFar = 6.0f;

	GetSize(&winSize, &winSize);
	SetSize(winSize, winSize);

	for(unsigned int i(0); i < 6; i++)
		ray[i] = 0.0f;

    m_numverts = 0;

    // Explicitly create a new rendering context instance for this canvas.
    m_glRC = new wxGLContext(this);
}

TestGLCanvas::~TestGLCanvas(){ delete m_glRC; }

void TestGLCanvas::newSurface(const std::vector<double> &surface){
	m_numverts = 0;
	for(unsigned int i(0); i < surface.size()/6 && m_numverts <= MAXVERTS; i++){
		unsigned int j(6*i);
		for(unsigned int k(0); k < 3; k++){
			m_verts[m_numverts][k] = surface[j + k];
			m_norms[m_numverts][k] = surface[j + 3 + k];
		}
		m_cols[m_numverts][0] = GLfloat(i)/GLfloat(surface.size()/6);
		m_cols[m_numverts][1] = GLfloat(surface.size()/6 - i)/GLfloat(surface.size()/6);
		m_cols[m_numverts][2] = GLfloat(i)/GLfloat(surface.size()/6);
		m_cols[m_numverts][3] = 0.5;
		m_numverts++;
	}
	Refresh(false);
}

void TestGLCanvas::newScaffold(const std::vector<double> &scaffold){
	m_numverts = 0;
	for(unsigned int i(0); i < scaffold.size()/7 && m_numverts <= MAXVERTS; i++, m_numverts++){
		for(unsigned int j(0); j < 3; j++){
			m_verts[m_numverts][j] = scaffold[7*i + j];
			m_cols[m_numverts][j] = scaffold[7*i + j + 3];
		}
		m_cols[m_numverts][3] = scaffold[7*i + 6];
	}
	Refresh(false);
}

void normalize(GLfloat &x, GLfloat &y, GLfloat &z){
	GLfloat mag(sqrt(x*x + y*y + z*z));
	x /= mag;
	y /= mag;
	z /= mag;
}

void TestGLCanvas::newPoints(const std::vector<double> &points){
	m_numverts = 0;
	for(unsigned int i(0); i < points.size()/7 && m_numverts <= MAXVERTS; i++, m_numverts++){
		for(unsigned int j(0); j < 3; j++){
			m_verts[m_numverts][j] = points[7*i + j];
			m_norms[m_numverts][j] = rkiss() - 0.5;
			m_cols[m_numverts][j] = points[7*i + j + 3];
		}
		normalize(m_norms[m_numverts][0], m_norms[m_numverts][1], m_norms[m_numverts][2]);
		m_cols[m_numverts][3] = points[7*i + 6];
	}
	Refresh(false);
}

void TestGLCanvas::newLines(const std::vector<double> &lines){
	m_numlineverts = 0;
	for(unsigned int i(0); i < lines.size()/7 && m_numlineverts <= MAXVERTS; i++, m_numlineverts++){
		for(unsigned int j(0); j < 3; j++){
			m_verts[m_numverts + m_numlineverts][j] = lines[7*i + j];
			m_cols[m_numverts + m_numlineverts][j] = lines[7*i + j + 3];
		}
		m_cols[m_numverts + m_numlineverts][3] = lines[7*i + 6];
	}
	Refresh(false);
}

void TestGLCanvas::addLine(const std::vector<double> &line){
	for(unsigned int i(0); i < line.size()/7 && m_numlineverts <= MAXVERTS; i++, m_numlineverts++){
		for(unsigned int j(0); j < 3; j++){
			m_verts[m_numverts + m_numlineverts][j] = line[7*i + j];
			m_cols[m_numverts + m_numlineverts][j] = line[7*i + j + 3];
		}
		m_cols[m_numverts + m_numlineverts][3] = line[7*i + 6];
	}
}

void TestGLCanvas::updateCols(std::map<unsigned int, std::vector<double> > &uc){
	for(std::map<unsigned int, std::vector<double> >::iterator it(uc.begin()); it != uc.end(); it++){
		unsigned int i(it->first);
		for(unsigned int j(0); j < 4; j++)
			m_cols[i][j] = it->second[j];
	}
	Refresh(false);
}

void TestGLCanvas::newNumbers(const std::vector<double> &numbers){
	m_numnumbers = 0;
	for(unsigned int i(0); i < numbers.size()/8 && m_numnumbers <= MAXVERTS; i++, m_numnumbers++){
		for(unsigned int j(0); j < 3; j++){
			m_numberverts[m_numnumbers][j] = numbers[8*i + j];
			m_numbercols[m_numnumbers][j] = numbers[8*i + j + 3];
		}
		normalize(m_norms[m_numnumbers][0], m_norms[m_numnumbers][1], m_norms[m_numnumbers][2]);
		m_numbercols[m_numnumbers][3] = numbers[8*i + 6];
		m_numbers[m_numnumbers] = numbers[8*i + 7];
	}
	Refresh(false);
}

void TestGLCanvas::addNumber(double w, double x, double y, double z, double r, double g, double b, double a){
	GLdouble numbers[] = {x, y, z, r, g, b, a, w};
	for(unsigned int j(0); j < 3; j++){
		m_numberverts[m_numnumbers][j] = numbers[j];
		m_numbercols[m_numnumbers][j] = numbers[j + 3];
	}
	normalize(m_norms[m_numnumbers][0], m_norms[m_numnumbers][1], m_norms[m_numnumbers][2]);
	m_numbercols[m_numnumbers][3] = numbers[6];
	m_numbers[m_numnumbers] = numbers[7];
	m_numnumbers++;
	Refresh(false);
}

// a x b = c
void crossProduct(GLfloat ax, GLfloat ay, GLfloat az, GLfloat bx, GLfloat by, GLfloat bz, GLfloat &cx, GLfloat &cy, GLfloat &cz){
	cx = ay*bz - az*by;
	cy = az*bx - ax*bz;
	cz = ax*by - ay*bx;
}

// a is eye, b is center, c is vector from eye to center
void ahead(GLfloat ax, GLfloat ay, GLfloat az, GLfloat bx, GLfloat by, GLfloat bz, GLfloat &cx, GLfloat &cy, GLfloat &cz){
	cx = bx - ax;
	cy = by - ay;
	cz = bz - az;
}

inline GLfloat magnitude(GLfloat x, GLfloat y, GLfloat z){
	return sqrt(x*x + y*y + z*z);
};

void TestGLCanvas::positionEye(double dx, double dy, double dz){

	GLfloat aheadx(0.0f), aheady(0.0f), aheadz(0.0f);
	ahead(m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, aheadx, aheady, aheadz);
	GLfloat r(magnitude(aheadx, aheady, aheadz));

	if(modifierIsDown){ // move center
		// approximate movement in tangent plane
		GLfloat scaleCoefx(0.01f), scaleCoefy(0.01f), scaleCoefz(0.01f);
		m_centerx += scaleCoefx*m_rightx*dx - scaleCoefy*m_upx*dy;
		m_centery += scaleCoefx*m_righty*dx - scaleCoefy*m_upy*dy;
		m_centerz += scaleCoefx*m_rightz*dx - scaleCoefy*m_upz*dy;

		// adjust separation of eye and center to make a rotation
		ahead(m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, aheadx, aheady, aheadz);
		GLfloat rNew(magnitude(aheadx, aheady, aheadz));
		m_centerx = m_eyex + r*aheadx/rNew;
		m_centery = m_eyey + r*aheady/rNew;
		m_centerz = m_eyez + r*aheadz/rNew;

		// move in direction of ahead (could be moved before updating dx and dy movement)
		normalize(aheadx, aheady, aheadz);
		m_centerx += scaleCoefz*aheadx*dz;
		m_centery += scaleCoefz*aheady*dz;
		m_centerz += scaleCoefz*aheadz*dz;
		m_eyex += scaleCoefz*aheadx*dz;
		m_eyey += scaleCoefz*aheady*dz;
		m_eyez += scaleCoefz*aheadz*dz;
	}else{ // move eye
		// approximate movement in tangent plane
		GLfloat scaleCoefx(0.1f), scaleCoefy(0.1f), scaleCoefz(0.1f);
		m_eyex -= scaleCoefx*m_rightx*dx - scaleCoefy*m_upx*dy;
		m_eyey -= scaleCoefx*m_righty*dx - scaleCoefy*m_upy*dy;
		m_eyez -= scaleCoefx*m_rightz*dx - scaleCoefy*m_upz*dy;
		ahead(m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, aheadx, aheady, aheadz);

		// adjust separation of eye and center to make a rotation
		GLfloat rNew(magnitude(m_centerx - m_eyex, m_centery - m_eyey, m_centerz - m_eyez));
		m_eyex = m_centerx - r*aheadx/rNew;
		m_eyey = m_centery - r*aheady/rNew;
		m_eyez = m_centerz - r*aheadz/rNew;

		// zoom eye
		m_zoom -= scaleCoefz*dz;
		double zoomFact(pow(10.0, m_zoom));
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		zNear = r - sqrt(3.0);
		zFar = r + sqrt(3.0);
		glFrustum(-zoomFact, zoomFact, -zoomFact, zoomFact, zNear, zFar);
		glFogf(GL_FOG_START, r - sqrt(3.0));
		glFogf(GL_FOG_END, r + sqrt(3.0));
	}

	// adjust up to be perpendicular to ahead
	ahead(m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, aheadx, aheady, aheadz);
	normalize(aheadx, aheady, aheadz);
	m_upx -= m_upx*aheadx;
	m_upy -= m_upy*aheady;
	m_upz -= m_upz*aheadz;
	normalize(m_upx, m_upy, m_upz);

	// update MODELVIEW
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(m_eyex, m_eyey, m_eyez, m_centerx, m_centery, m_centerz, m_upx, m_upy, m_upz);
	
	// adjust right to be perpendicular to ahead and up
	crossProduct(aheadx, aheady, aheadz, m_upx, m_upy, m_upz, m_rightx, m_righty, m_rightz);
	normalize(m_rightx, m_righty, m_rightz);
}

string selectedString(){
	streamsize precDefault(4);
	double len(backboneLength(backbonesGlobal[ccSelected][bbSelected])),
		vol(voxdimsGlobal[0]*voxdimsGlobal[1]*voxdimsGlobal[2]*volumesGlobal[ccSelected][bbSelected]);
	return "cc " + makeString(ccSelected) + ", bb " + makeString(bbSelected) + ", v " + makeString(vertSelected)
		+ ", vol=" + makeString(vol, precDefault) + "cu." + lengthUnitGlobal
		+ " (" + makeString(volumesGlobal[ccSelected][bbSelected]) + ")"
		+ " l=" + makeString(len, precDefault) + lengthUnitGlobal
		+ " <r_lv>=" + makeString(radFromVolLen(vol, len), precDefault) + lengthUnitGlobal
		+ " <r_so>=" + makeString(aveRadSolidGlobal[ccSelected][bbSelected], precDefault) + lengthUnitGlobal
		+ " <r_su>=" + makeString(aveRadSurfGlobal[ccSelected][bbSelected], precDefault) + lengthUnitGlobal;
}

void writeNumber(string s, double x, double y, double z, double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0){
	glColor4d(r, g, b, a);
	glRasterPos3d(x, y, z);
	if(labelStatus%3 == 2){
		for(unsigned int i(0); i < s.length(); i++){
			switch(s[i]){
				case '-':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitDash16f); break;
				case '.':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitPeriod16f); break;
				case '0':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitZero16f); break;
				case '1':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitOne16f); break;
				case '2':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitTwo16f); break;
				case '3':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitThree16f); break;
				case '4':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitFour16f); break;
				case '5':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitFive16f); break;
				case '6':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitSix16f); break;
				case '7':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitSeven16f); break;
				case '8':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitEight16f); break;
				case '9':
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitNine16f); break;
				default:
					glBitmap(16, 16, 0.0f, 0.0f, 16.0f, 0.0f, bitSpace16f); break;
			}
		}
	}else if(labelStatus%3 == 0){
		for(unsigned int i(0); i < s.length(); i++){
			switch(s[i]){
				case '-':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitDash8f); break;
				case '.':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitPeriod8f); break;
				case '0':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitZero8f); break;
				case '1':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitOne8f); break;
				case '2':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitTwo8f); break;
				case '3':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitThree8f); break;
				case '4':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitFour8f); break;
				case '5':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitFive8f); break;
				case '6':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitSix8f); break;
				case '7':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitSeven8f); break;
				case '8':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitEight8f); break;
				case '9':
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitNine8f); break;
				default:
					glBitmap(8, 8, 0.0f, 0.0f, 8.0f, 0.0f, bitSpace8f); break;
			}
		}
	}
}

void writeNaturalNumber(unsigned int n, double x, double y, double z, double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0){
	writeNumber(makeString(n, 8), x, y, z, r, g, b, a);
}

void writeNumber(double w, double x, double y, double z, double r = 0.0, double g = 0.0, double b = 0.0, double a = 1.0){
	writeNumber(makeString(w, 8), x, y, z, r, g, b, a);
}

void TestGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event)){
    // This is a dummy, to avoid an endless succession of paint messages.
    // OnPaint handlers must always create a wxPaintDC.
    wxPaintDC dc(this);

    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent(*m_glRC);

	

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();

    // draw
    if (g_use_vertex_arrays){
		glDrawArrays(GL_POINTS, 0, m_numverts);
		glDrawArrays(GL_LINES, m_numverts, m_numlineverts);
	}else{
        glBegin(GL_POINTS);
        for(int i(0); i < m_numverts; i++){
			glColor4fv(m_cols[i]);
            glNormal3fv(m_norms[i]);
            glVertex3fv(m_verts[i]);
        }
        glEnd();
		glBegin(GL_LINES);
		for(int i(m_numverts); i < m_numverts + m_numlineverts; i++){
			glColor4fv(m_cols[i]);
            glVertex3fv(m_verts[i]);
        }
		glEnd();
    }

	// point at focus
	glBegin(GL_POINTS);
	glColor4dv(centerColor);
	glVertex3d(m_centerx, m_centery, m_centerz);
	glEnd();

	// trace ray
	if(ray[0] == ray[3] || ray[1] == ray[4] || ray[2] == ray[5]){
		glBegin(GL_POINTS);
		glColor4dv(selectColor);
		glVertex3dv(&ray[0]);
		glEnd();
	}else{
		glBegin(GL_LINES);
		glColor4dv(selectColor);
		glVertex3dv(&ray[0]);
		glColor4dv(selectColor);
		glVertex3dv(&ray[3]);
		glEnd();
		glBegin(GL_POINTS);
		glColor4dv(selectColor);
		glVertex3d((ray[3] + ray[0])/2.0, (ray[4] + ray[1])/2.0, (ray[5] + ray[2])/2.0);
		glEnd();
	}

	// selected segment
	if(segIsSelected){
		glBegin(GL_POINTS);
		for(unsigned int i(0); i < backbonesGlobal[ccSelected][bbSelected].size(); i++){
			double nx(0.0), ny(0.0), nz(0.0);
			normalizedCoord(backbonesGlobal[ccSelected][bbSelected][i], nx, ny, nz);
			double offsetInc(0.01);
			if(i%3 == 0){
				glColor4dv(highlightColor1);
				glVertex3d(nx + offsetInc, ny, nz);
				glColor4dv(highlightColor1);
				glVertex3d(nx - offsetInc, ny, nz);
			}
			else if(i%3 == 1){
				glColor4dv(highlightColor2);
				glVertex3d(nx, ny + offsetInc, nz);
				glColor4dv(highlightColor2);
				glVertex3d(nx, ny - offsetInc, nz);
			}else{
				glColor4dv(highlightColor3);
				glVertex3d(nx, ny, nz + offsetInc);
				glColor4dv(highlightColor3);
				glVertex3d(nx, ny, nz - offsetInc);
			}
			glVertex3d(nx, ny, nz);
		}
		glEnd();
	}
	if(updateSelectedStatus){
		updateSelectedStatus = false;
		if(segIsSelected){
			selectedStatusString = selectedString();
			wxGetApp().myFrame->updateStatus("Selected");
		}else
			wxGetApp().myFrame->updateStatus("");
	}

	GLfloat position[] = {m_eyex, m_eyey, m_eyez, 1.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	// numbers
	for(int i(0); i < m_numnumbers; i++){
		writeNumber(m_numbers[i], m_numberverts[i][0], m_numberverts[i][1], m_numberverts[i][2],
			m_numbercols[i][0], m_numbercols[i][1], m_numbercols[i][2], m_numbercols[i][3]);
	}

    glPopMatrix();
    SwapBuffers();
}


int TestGLCanvas::getNewSize(int oldSize, int x, int y){
	if(x < y)
		return x;
	return y;
}

void TestGLCanvas::OnSize(wxSizeEvent& event)
{
    if (!IsShownOnScreen())
        return;
    // This is normally only necessary if there is more than one wxGLCanvas
    // or more than one wxGLContext in the application.
    SetCurrent(*m_glRC);

	int newSize(getNewSize(winSize, event.GetSize().x, event.GetSize().y));
	//SetSize(newSize, newSize);

    // It's up to the application code to update the OpenGL viewport settings.
    // This is OK here only because there is only one canvas that uses the
    // context. See the cube sample for that case that multiple canvases are
    // made current with one context.
    // glViewport(0, 0, event.GetSize().x, event.GetSize().y);
	glViewport(0, 0, newSize, newSize);
	winSize = newSize;

	// doh wants a Refresh!
	Refresh(false);
}

void TestGLCanvas::OnChar(wxKeyEvent& event){

    switch(event.GetKeyCode()){
    
	//case WXK_ESCAPE:
    //    wxTheApp->ExitMainLoop();
    //    return;

    case WXK_LEFT:
        m_yrot -= 15.0;
        break;

    case WXK_RIGHT:
        m_yrot += 15.0;
        break;

    case WXK_UP:
        m_xrot += 15.0;
        break;

    case WXK_DOWN:
        m_xrot -= 15.0;
        break;

    case 'l': case 'L':
		if(modifierIsDown)
			labelStatus += 2;
		else
			labelStatus++;
		break;

    default:
        event.Skip();
        return;
    }

    Refresh(false);
}

void TestGLCanvas::OnKeyDown(wxKeyEvent& event){
	
	switch(event.GetKeyCode()){
    
	case WXK_SHIFT:
        modifierIsDown = true;
        Refresh(false);
        return;

    default:
        event.Skip();
        return;
    }
	
}

void TestGLCanvas::OnKeyUp(wxKeyEvent& event){
	
	switch(event.GetKeyCode()){
    
	case WXK_SHIFT:
        modifierIsDown = false;
        Refresh(false);
        return;
	
    default:
        event.Skip();
        return;
    }
	
}

void TestGLCanvas::updateTranslation(double dx, double dy, double dz){
	double radPerDeg(acos(-1.0)/180.0);
	double DX(dx*cos(m_yrot*radPerDeg) + dy*sin(m_xrot*radPerDeg)),
		DY(dy*cos(m_xrot*radPerDeg) + dx*sin(m_yrot*radPerDeg)),
		DZ(dz);
	m_xtrans += 0.01*DX;
	m_ytrans -= 0.01*DY;
	m_ztrans += 0.01*DZ;
}

void TestGLCanvas::processClick(float x, float y){
	int w, h;
	GetSize(&w, &h);
	y = h - y;
	GLdouble model[16], proj[16];
	GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	if(ray[0] == ray[3] && ray[1] == ray[4] && ray[2] == ray[5]){
		segIsSelected = false;
		selectedStatusString = "";
		GLint suc = gluUnProject(x, y, 0.0f, model, proj, viewport, &ray[0], &ray[1], &ray[2]);
		suc = suc && gluUnProject(x, y, 1.0f, model, proj, viewport, &ray[3], &ray[4], &ray[5]);
	}else{
		GLdouble x0(0.0), y0(0.0), z0(0.0), x1(0.0), y1(0.0), z1(0.0);
		GLint suc = gluUnProject(x, y, -1.0f, model, proj, viewport, &x0, &y0, &z0);
		suc = suc && gluUnProject(x, y, 1.0f, model, proj, viewport, &x1, &y1, &z1);
        GLfloat m[3], n[3], b[3], c[3];
        for(unsigned int i(0); i < 3; i++){
            m[i] = ray[i + 3] - ray[i];
            b[i] = ray[i];
        }
        n[0] = x1 - x0;
        n[1] = y1 - y0;
        n[2] = z1 - z0;
        c[0] = x0;
        c[1] = y0;
        c[2] = z0;
		normalize(m[0], m[1], m[2]);
		normalize(n[0], n[1], n[2]);
		float mn(0.0), dm(0.0), ys(0.0);
		for(unsigned int i(0); i < 3; i++)
			mn += m[i]*n[i];
		if(abs(mn) >= 1.0) // parallel
			return;
		for(unsigned int i(0); i < 3; i++){
			dm += (b[i] - c[i])*m[i];
			ys += (b[i] - c[i])*(mn*m[i] - n[i]);
		}
		ys /= mn*mn - 1.0;
		for(unsigned int i(0); i < 3; i++)
			ray[i] = ray[i + 3] = (m[i]*(mn*ys - dm) + n[i]*ys + b[i] + c[i])/2.0;
		futSelected = std::async(findClosestSegment, ray[0], ray[1], ray[2]);
	}
	updateSelectedStatus = true;
	Refresh(false);
}

void TestGLCanvas::OnMouseEvent(wxMouseEvent& event){
    static int dragging = 0;
	static float last_x, last_y, first_x, first_y;

    // Allow default processing to happen, or else the canvas cannot gain focus
    // (for key events).
    event.Skip();

	if(event.LeftIsDown()){
		if (!dragging){
			first_x = event.GetX();
			first_y = event.GetY();
			dragging = 1;
		}
		else{
			//updateTranslation(event.GetX() - last_x, event.GetY() - last_y, 0.0);
			positionEye(event.GetX() - last_x, event.GetY() - last_y, 0.0);
			
		}
		last_x = event.GetX();
		last_y = event.GetY();
	}else{
		if(wxGetApp().myFrame != NULL && first_x == event.GetX() && first_y == event.GetY())
			processClick(first_x, first_y);
		dragging = 0;
	}
	
	if(event.GetWheelRotation() != 0)
		positionEye(0.0, 0.0, 1.0*event.GetWheelRotation()/event.GetWheelDelta());

	Refresh(false);
}


void TestGLCanvas::InitMaterials(){
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(5);
	glPointSize(4);
}

void TestGLCanvas::InitGL(){
    // Make the new context current (activate it for use) with this canvas.
    SetCurrent(*m_glRC);

    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    InitMaterials();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -zNear - 1.0 );

    if (g_use_vertex_arrays){
        glVertexPointer(3, GL_FLOAT, 0, m_verts);
        glNormalPointer(GL_FLOAT, 0, m_norms);
		glColorPointer(4, GL_FLOAT, 0, m_cols);

		glVertexPointer(3, GL_FLOAT, m_numverts, m_verts);
		glColorPointer(4, GL_FLOAT, m_numverts, m_cols);

        glEnable(GL_VERTEX_ARRAY);
        glEnable(GL_NORMAL_ARRAY);
		glEnable(GL_COLOR_ARRAY);

		
    }

	glEnable(GL_FOG);
	//glFogf(GL_FOG_DENSITY, 0.5f);
	glFogfv(GL_FOG_COLOR, clearColor);
	glFogi(GL_FOG_MODE, GL_LINEAR);

	positionEye(0.0, 0.0, 0.0);

    InitMaterials();
}

#endif


