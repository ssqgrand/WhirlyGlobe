/*
 *  VectorData.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/11.
 *  Copyright 2011-2017 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <UIKit/UIKit.h>
#import <math.h>
#import <vector>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "CoordSystem.h"

namespace WhirlyKit
{
	
/// The base class for vector shapes.  All shapes
///  have attribute and an MBR.
class VectorShape : public Identifiable
{
public:	
	/// Set the attribute dictionary
	void setAttrDict(NSMutableDictionary *newDict);
	
	/// Return the attr dict
	NSMutableDictionary *getAttrDict();    
    /// Return the geoMbr
    virtual GeoMbr calcGeoMbr() = 0;
	
protected:
	VectorShape();
	virtual ~VectorShape();

	__strong NSMutableDictionary *attrDict;
};

class VectorAreal;
class VectorLinear;
class VectorLinear3d;
class VectorPoints;
class VectorTriangles;

/// Reference counted version of the base vector shape
typedef std::shared_ptr<VectorShape> VectorShapeRef;
/// Reference counted Areal
typedef std::shared_ptr<VectorAreal> VectorArealRef;
/// Reference counted Linear
typedef std::shared_ptr<VectorLinear> VectorLinearRef;
/// Reference counted Linear3d
typedef std::shared_ptr<VectorLinear3d> VectorLinear3dRef;
/// Reference counted Points
typedef std::shared_ptr<VectorPoints> VectorPointsRef;
/// Reference counted triangle mesh
typedef std::shared_ptr<VectorTriangles> VectorTrianglesRef;

/// Vector Ring is just a vector of 2D points
typedef std::vector<Point2f> VectorRing;
    
/// Vector Ring of 3D doubles
typedef std::vector<Point3d> VectorRing3d;

/// Comparison function for the vector shape.
/// This is here to ensure we don't put in the same pointer twice
struct VectorShapeRefCmp
{
    bool operator()(const VectorShapeRef &a,const VectorShapeRef &b) const
    { return a.get() < b.get(); }
};
  
/// We pass the shape set around when returing a group of shapes.
/// It's a set of reference counted shapes.  You have to dynamic
///  cast to get the specfic type.  Don't forget to use the std dynamic cast
typedef std::set<VectorShapeRef,VectorShapeRefCmp> ShapeSet;
    
/// Calculate area of a loop
float CalcLoopArea(const VectorRing &);
/// Calculate area of a loop
double CalcLoopArea(const std::vector<Point2d> &);
/// Calculate the centroid of a loop
Point2f CalcLoopCentroid(const VectorRing &loop);
/// Calculate the centroid of a bunch of points
Point2d CalcLoopCentroid(const std::vector<Point2d> &loop);
/// Calculate the center of mass of the points
Point2d CalcCenterOfMass(const std::vector<Point2d> &loop);
    
/// Collection of triangles forming a mesh
class VectorTriangles : public VectorShape
{
public:
    /// Creation function.  Use this instead of new.
    static VectorTrianglesRef createTriangles();
    ~VectorTriangles();
    
    /// Simple triangle with three points (obviously)
    typedef struct
    {
    public:
        int pts[3];
    } Triangle;
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();
    
    // Return the given triangle as a VectorRing
    void getTriangle(int which,VectorRing &ring);
    
    /// True if the given point is within one of the triangles
    bool pointInside(GeoCoord coord);
    
    // Bounding box in 2D
	GeoMbr geoMbr;

    // Shared points
    std::vector<Point3f> pts;
    // Triangles
    std::vector<Triangle> tris;
    
protected:
    VectorTriangles();
};

/// Look for a triangle/ray intersection in the mesh
bool VectorTrianglesRayIntersect(const Point3d &org,const Point3d &dir,const VectorTriangles &mesh,double *outT,Point3d *iPt);

/// Areal feature is a list of loops.  The first is an outer loop
///  and all the rest are inner loops
class VectorAreal : public VectorShape
{
public:
    /// Creation function.  Use this instead of new
    static VectorArealRef createAreal();
    ~VectorAreal();
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();
    
    /// True if the given point is within one of the loops
    bool pointInside(GeoCoord coord);
    
    /// Sudivide to the given tolerance (in degrees)
    void subdivide(float tolerance);
        
    /// Bounding box in geographic coordinates.
	GeoMbr geoMbr;
	std::vector<VectorRing> loops;
    
protected:
    VectorAreal();
};

/// Linear feature is just a list of points that form
///  a set of edges
class VectorLinear : public VectorShape
{
public:
    /// Creation function.  Use instead of new
    static VectorLinearRef createLinear();
    ~VectorLinear();
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();

    /// Sudivide to the given tolerance (in degrees)
    void subdivide(float tolerance);

	GeoMbr geoMbr;
	VectorRing pts;
    
protected:
    VectorLinear();
};

/// Linear feature is just a list of points that form
///  a set of edges.  This version has z as well.
class VectorLinear3d : public VectorShape
{
public:
    /// Creation function.  Use instead of new
    static VectorLinear3dRef createLinear();
    ~VectorLinear3d();
    
    virtual GeoMbr calcGeoMbr();
    void initGeoMbr();
        
    GeoMbr geoMbr;
    VectorRing3d pts;
    
protected:
    VectorLinear3d();
};

/// The Points feature is a list of points that share attributes
///  and are otherwise unrelated.  In most cases you'll get one
///  point, but be prepared for multiple.
class VectorPoints : public VectorShape
{
public:
    /// Creation function.  Use instead of new
    static VectorPointsRef createPoints();
    ~VectorPoints();
    
    /// Return the bounding box
    virtual GeoMbr calcGeoMbr();
    
    /// Calculate the bounding box from data
    void initGeoMbr();

	GeoMbr geoMbr;
	VectorRing pts;
    
protected:
    VectorPoints();
};
    
/// A set of strings
typedef std::set<std::string> StringSet;
    
/// Break any edge longer than the given length.
/// Returns true if it broke anything
void SubdivideEdges(const VectorRing &inPts,VectorRing &outPts,bool closed,float maxLen);
void SubdivideEdges(const VectorRing3d &inPts,VectorRing3d &outPts,bool closed,float maxLen);

/// Break any edge that deviates by the given epsilon from the surface described in
/// the display adapter;
void SubdivideEdgesToSurface(const VectorRing &inPts,VectorRing &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps);
void SubdivideEdgesToSurface(const VectorRing3d &inPts,VectorRing3d &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps);

/// Break any edge that deviates by the given epsilon from the surface described in
///  the display adapter.  But rather than using lat lon values, we'll output in
///  display coordinates and build points along the great circle.
void SubdivideEdgesToSurfaceGC(const VectorRing &inPts,VectorRing3d &outPts,bool closed,CoordSystemDisplayAdapter *adapter,float eps,float sphereOffset = 0.0,int minPts = 0);

/** Base class for loading a vector data file.
    Fill this into hand data over to whomever wants it.
 */
class VectorReader
{
public:
    VectorReader() { }
    virtual ~VectorReader() { }
	
    /// Return false if we failed to load
    virtual bool isValid() = 0;
	
    /// Returns one of the vector types.
    /// Keep enough state to figure out what the next one is.
    /// You can skip any attributes not named in the filter.  Or just ignore it.
    virtual VectorShapeRef getNextObject(const StringSet *filter) = 0;
    
    /// Return true if this vector reader can seek and read
    virtual bool canReadByIndex() { return false; }
    
    /// Return the total number of vectors objects
    virtual unsigned int getNumObjects() { return 0; }
    
    /// Return an object that corresponds to the given index.
    /// You need to be able to seek in your file format for this.
    /// The filter works the same as for getNextObect()
    virtual VectorShapeRef getObjectByIndex(unsigned int vecIndex,const StringSet *filter)  { return VectorShapeRef(); }
};

/** Helper routine to parse geoJSON into a collection of vectors.
    We don't know for sure what we'll get back, so you have to go
    looking through it.  Return false on parse failure.
 */
bool VectorParseGeoJSON(ShapeSet &shapes,NSData *jsonData,NSString **crs);
 
/** Helper routine to parse geoJSON into a collection of vectors.
    We don't know for sure what we'll get back, so you have to go
    looking through it.  Return false on parse failure.
 */
bool VectorParseGeoJSON(ShapeSet &shapes,NSDictionary *jsonDict);
    
/** Helper routine to parse a GeoJSON assembly into an array of
    collections of vectors.  This format is returned by the experimental
    OSM server for vectors.
  */
bool VectorParseGeoJSONAssembly(NSData *data,std::map<std::string,ShapeSet> &shapes);
    
bool VectorReadFile(const std::string &fileName,ShapeSet &shapes);
bool VectorWriteFile(const std::string &fileName,ShapeSet &shapes);
    
}

