#include "surf.h"
#include "extra.h"
using namespace std;

namespace
{
    
    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                return false;
    
        return true;
    }
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface;
    
    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

    unsigned profileSize = profile.size();
    
    for(unsigned step = 0; step <= steps; step++)
    {
        float angle = 2.0f*M_PI*step/steps;
        float cosA = cos(angle);
        float sinA = sin(angle);

        for(unsigned i = 0; i < profileSize; i++)
        {
            float x = profile[i].V[0];
            float y = profile[i].V[1];

            surface.VV.push_back(Vector3f(x*cosA, y, x*sinA));

            float nx = profile[i].N[0];
            float ny = profile[i].N[1];

            surface.VN.push_back(Vector3f(-nx*cosA, -ny, -nx*sinA).normalized());
        }
    }

    //Generate Triangle faces

    for(unsigned step = 0; step < steps; step++)
    {
        for(unsigned i = 0; i < profileSize - 1; i++)
        {
            unsigned a = step * profileSize + i;
            unsigned b = (step + 1) * profileSize + i;
            unsigned c = (step + 1) * profileSize + i + 1;
            unsigned d = step * profileSize + i + 1;

            surface.VF.push_back(Tup3u(a, c, b));
            surface.VF.push_back(Tup3u(a, d, c));
        }
    }

    return surface;

    // cerr << "\t>>> makeSurfRev called (but not implemented).\n\t>>> Returning empty surface." << endl;
 
    // return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep )
{
    Surface surface;

    if (!checkFlat(profile))
    {
        cerr << "genCyl profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // TODO: Here you should build the surface.  See surf.h for details.

    unsigned profileSize = profile.size();
    unsigned sweepSize = sweep.size();

    for(unsigned s = 0; s < sweepSize; s++)
    {
        Vector3f N = sweep[s].N;
        Vector3f B = sweep[s].B;
        Vector3f V = sweep[s].V;

        for(unsigned i = 0; i < profileSize - 1; i++)
        {
            float px = profile[i].V[0];
            float py = profile[i].V[1];

            surface.VV.push_back(V + px*N + py*B);

            float nx = profile[i].N[0];
            float ny = profile[i].N[1];
            surface.VN.push_back((nx*N + ny*B).normalized());
        }
    }

    for(unsigned s = 0; s < sweepSize - 1; s++)
    {
        for(unsigned i = 0; i < profileSize; i++)
        {
            unsigned a = s * profileSize + i;
            unsigned b = (s + 1) * profileSize + i;
            unsigned c = (s + 1) * profileSize + i + 1;
            unsigned d = s * profileSize + i + 1;

            surface.VF.push_back(Tup3u(a, c, b));
            surface.VF.push_back(Tup3u(a, d, c));
        }
    }

    return surface;

    // cerr << "\t>>> makeGenCyl called (but not implemented).\n\t>>> Returning empty surface." <<endl;

    // return surface;
}

void drawSurface(const Surface &surface, bool shaded)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (shaded)
    {
        // This will use the current material color and light
        // positions.  Just set these in drawScene();
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // This tells openGL to *not* draw backwards-facing triangles.
        // This is more efficient, and in addition it will help you
        // make sure that your triangles are drawn in the right order.
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {        
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glColor4f(0.4f,0.4f,0.4f,1.f);
        glLineWidth(1);
    }

    glBegin(GL_TRIANGLES);
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        glNormal(surface.VN[surface.VF[i][0]]);
        glVertex(surface.VV[surface.VF[i][0]]);
        glNormal(surface.VN[surface.VF[i][1]]);
        glVertex(surface.VV[surface.VF[i][1]]);
        glNormal(surface.VN[surface.VF[i][2]]);
        glVertex(surface.VV[surface.VF[i][2]]);
    }
    glEnd();

    glPopAttrib();
}

void drawNormals(const Surface &surface, float len)
{
    // Save current state of OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glDisable(GL_LIGHTING);
    glColor4f(0,1,1,1);
    glLineWidth(1);

    glBegin(GL_LINES);
    for (unsigned i=0; i<surface.VV.size(); i++)
    {
        glVertex(surface.VV[i]);
        glVertex(surface.VV[i] + surface.VN[i] * len);
    }
    glEnd();

    glPopAttrib();
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (unsigned i=0; i<surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (unsigned i=0; i<surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}
