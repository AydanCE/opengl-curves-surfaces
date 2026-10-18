#include "curve.h"
#include "extra.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
using namespace std;

namespace
{
    // Approximately equal to.  We don't want to use == because of
    // precision issues with floating point.
    inline bool approx( const Vector3f& lhs, const Vector3f& rhs )
    {
        const float eps = 1e-8f;
        return ( lhs - rhs ).absSquared() < eps;
    }

    
}
    

Curve evalBezier( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 || P.size() % 3 != 1 )
    {
        cerr << "evalBezier must be called with 3n+1 control points." << endl;
        exit( 0 );
    }

    // TODO:
    // You should implement this function so that it returns a Curve
    // (e.g., a vector< CurvePoint >).  The variable "steps" tells you
    // the number of points to generate on each piece of the spline.
    // At least, that's how the sample solution is implemented and how
    // the SWP files are written.  But you are free to interpret this
    // variable however you want, so long as you can control the
    // "resolution" of the discretized spline curve with it.

    // Make sure that this function computes all the appropriate
    // Vector3fs for each CurvePoint: V,T,N,B.
    // [NBT] should be unit and orthogonal.

    // Also note that you may assume that all Bezier curves that you
    // receive have G1 continuity.  Otherwise, the TNB will not be
    // be defined at points where this does not hold.

    Curve curve;
    Vector3f prevB = Vector3f(0, 0, 1);

    for (unsigned i = 0; i + 3 < P.size(); i+=3)
    {
        Vector3f p0 = P[i];
        Vector3f p1 = P[i+1];
        Vector3f p2 = P[i+2];
        Vector3f p3 = P[i+3];

        for (unsigned s = 0; s < steps; s++)
        {
            float t = float(s)/steps;
            float b0 = (1-t)*(1-t)*(1-t);
            float b1 = 3*(1-t)*(1-t)*t;
            float b2 = 3*(1-t)*t*t;
            float b3 = t*t*t;

            Vector3f V = b0*p0 + b1*p1 + b2*p2 + b3*p3;
            float db0 = -3*(1-t)*(1-t);
            float db1 = 3*(1-t)*(1-t) - 6*(1-t)*t;
            float db2 = 6*(1-t)*t - 3*t*t;
            float db3 = 3*t*t;

            Vector3f T = (db0*p0 + db1*p1 + db2*p2 + db3*p3).normalized();

            Vector3f N = Vector3f::cross(prevB, T).normalized();

            if( N.absSquared() < 1e-6 )
            {
                // pick a different arbitrary vector
                Vector3f arbitrary = (fabs(T[0]) < 0.9f) ? Vector3f(1,0,0) : Vector3f(0,1,0);
                N = Vector3f::cross(arbitrary, T).normalized();
            }

            Vector3f B = Vector3f::cross(T, N).normalized();
            prevB = B;

            CurvePoint cp;
            cp.V = V;
            cp.T = T;
            cp.N = N;
            cp.B = B;
            curve.push_back(cp);
        }
    }

    unsigned last = P.size() - 4;
    CurvePoint lastPt;
    lastPt.V = P.back();

    Vector3f p0 = P[last], p1 = P[last+1], p2 = P[last+2], p3 = P[last+3];
    Vector3f T = (3*(p3 - 3*p2 + 3*p1 - p0)).normalized();
    Vector3f N = Vector3f::cross(prevB, T).normalized();
    Vector3f B = Vector3f::cross(T, N).normalized();

    lastPt.T = T;
    lastPt.N = N;
    lastPt.B = B;
    curve.push_back(lastPt);

    return curve;

    // cerr << "\t>>> evalBezier has been called with the following input:" << endl;

    // cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    // for( unsigned i = 0; i < P.size(); ++i )
    // {
    //     cerr << "\t>>> " << P[i] << endl;
    // }

    // cerr << "\t>>> Steps (type steps): " << steps << endl;
    // cerr << "\t>>> Returning empty curve." << endl;

    // // Right now this will just return this empty curve.
    // return Curve();
}

Curve evalBspline( const vector< Vector3f >& P, unsigned steps )
{
    // Check
    if( P.size() < 4 )
    {
        cerr << "evalBspline must be called with 4 or more control points." << endl;
        exit( 0 );
    }

    // TODO:
    // It is suggested that you implement this function by changing
    // basis from B-spline to Bezier.  That way, you can just call
    // your evalBezier function.

    Curve curve;

    for(unsigned seg = 0; seg + 3 < P.size(); seg++)
    {
        Vector3f p0 = P[seg];
        Vector3f p1 = P[seg+1];
        Vector3f p2 = P[seg+2];
        Vector3f p3 = P[seg+3];

        Vector3f b0 = (1.0f/6.0f) * (p0 + 4*p1 + p2);
        Vector3f b1 = (1.0f/6.0f) * (4*p1 + 2*p2);
        Vector3f b2 = (1.0f/6.0f) * (2*p1 + 4*p2);
        Vector3f b3 = (1.0f/6.0f) * (p1 + 4*p2 + p3);

        vector<Vector3f> bezierPts = {b0, b1, b2, b3};
        Curve segCurve = evalBezier(bezierPts, steps);

        for(unsigned i = 0; i < segCurve.size()-1; i++)
        {
            curve.push_back(segCurve[i]);
        }
    }

    if(!curve.empty())
    {
        unsigned last = P.size() - 4;
        Vector3f p1 = P[last+1], p2 = P[last+2], p3 = P[last+3];
        Vector3f b3 = (1.0f/6.0f) * (p1 + 4*p2 + p3);

        vector<Vector3f> bezierPts = {
            (1.0f/6.0f) * (P[last] + 4*P[last+1] + P[last+2]),
            (1.0f/6.0f) * (4*P[last+1] + 2*P[last+2]),
            (1.0f/6.0f) * (2*P[last+1] + 4*P[last+2]),
            b3
        };
        Curve lastSeg = evalBezier(bezierPts, steps);
        curve.push_back(lastSeg.back());
    }

    return curve;

    // cerr << "\t>>> evalBSpline has been called with the following input:" << endl;

    // cerr << "\t>>> Control points (type vector< Vector3f >): "<< endl;
    // for( unsigned i = 0; i < P.size(); ++i )
    // {
    //     cerr << "\t>>> " << P[i] << endl;
    // }

    // cerr << "\t>>> Steps (type steps): " << steps << endl;
    // cerr << "\t>>> Returning empty curve." << endl;

    // // Return an empty curve right now.
    // return Curve();
}

Curve evalCircle( float radius, unsigned steps )
{
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector< CurvePoint >).
    
    // Preallocate a curve with steps+1 CurvePoints
    Curve R( steps+1 );

    // Fill it in counterclockwise
    for( unsigned i = 0; i <= steps; ++i )
    {
        // step from 0 to 2pi
        float t = 2.0f * M_PI * float( i ) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vector3f( cos(t), sin(t), 0 );
        
        // Tangent vector is first derivative
        R[i].T = Vector3f( -sin(t), cos(t), 0 );
        
        // Normal vector is second derivative
        R[i].N = Vector3f( -cos(t), -sin(t), 0 );

        // Finally, binormal is facing up.
        R[i].B = Vector3f( 0, 0, 1 );
    }

    return R;
}

void drawCurve( const Curve& curve, float framesize )
{
    // Save current state of OpenGL
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    // Setup for line drawing
    glDisable( GL_LIGHTING ); 
    glColor4f( 1, 1, 1, 1 );
    glLineWidth( 1 );
    
    // Draw curve
    glBegin( GL_LINE_STRIP );
    for( unsigned i = 0; i < curve.size(); ++i )
    {
        glVertex( curve[ i ].V );
    }
    glEnd();

    glLineWidth( 1 );

    // Draw coordinate frames if framesize nonzero
    if( framesize != 0.0f )
    {
        Matrix4f M;

        for( unsigned i = 0; i < curve.size(); ++i )
        {
            M.setCol( 0, Vector4f( curve[i].N, 0 ) );
            M.setCol( 1, Vector4f( curve[i].B, 0 ) );
            M.setCol( 2, Vector4f( curve[i].T, 0 ) );
            M.setCol( 3, Vector4f( curve[i].V, 1 ) );

            glPushMatrix();
            glMultMatrixf( M );
            glScaled( framesize, framesize, framesize );
            glBegin( GL_LINES );
            glColor3f( 1, 0, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 1, 0, 0 );
            glColor3f( 0, 1, 0 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 1, 0 );
            glColor3f( 0, 0, 1 ); glVertex3d( 0, 0, 0 ); glVertex3d( 0, 0, 1 );
            glEnd();
            glPopMatrix();
        }
    }
    
    // Pop state
    glPopAttrib();
}

