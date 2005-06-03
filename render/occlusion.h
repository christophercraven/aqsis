// Aqsis
// Copyright � 1997 - 2002, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Declares the hierarchical occlusion culling class.
		\author Andy Gill (billybobjimboy@users.sf.net)
*/

//? Is .h included already?
#ifndef OCCLUSION_H_INCLUDED
#define OCCLUSION_H_INCLUDED 1

#include "aqsis.h"
#include "kdtree.h"
#include "imagepixel.h"

START_NAMESPACE( Aqsis )

class CqBound;
class CqBucket;


/**	\brief	The CqOcclusionKDTreeData class
	Specialisation of the KDTree data class to support generation of a KDTree
	representing the sample data of a bucket.
*/
class CqOcclusionTree
{
    class CqOcclusionTreeComparator
    {
    public:
        CqOcclusionTreeComparator(TqInt dimension) : m_Dim( dimension )
        {}

        bool operator()(SqSampleData* a, SqSampleData* b);

    private:
        TqInt		m_Dim;
    };

public:
    CqOcclusionTree(TqInt dimension = 0) : m_Dimension(dimension), m_Parent(0)
    {}

    void SortElements(TqInt dimension)
    {
        std::sort(m_Samples.begin(), m_Samples.end(), CqOcclusionTreeComparator(dimension) );
    }
    TqInt Dimensions() const	{return(2);}
	std::vector<SqSampleData*>& Samples()
	{
		return(m_Samples);
	}

	void ConstructTree();

	void PropagateChanges();

	void InitialiseBounds();
	void UpdateBounds();

//private:
	CqOcclusionTree* m_Parent;
	TqInt		m_Dimension;
	CqVector2D	m_MinSamplePoint;
	CqVector2D	m_MaxSamplePoint;
	TqFloat		m_MinTime;
	TqFloat		m_MaxTime;
	TqFloat		m_MaxOpaqueZ;
	TqInt		m_MinDofBoundIndex;
	TqInt		m_MaxDofBoundIndex;
	std::vector<CqOcclusionTree*>	m_Children;
	std::vector<SqSampleData*>		m_Samples;
};


class CqOcclusionBox
{
public:
    static void CreateHierarchy( TqInt bucketXSize, TqInt bucketYSize, TqInt XFWidth, TqInt YFWidth );
    static void DeleteHierarchy();
    static void SetupHierarchy( CqBucket* bucket, TqInt xMin, TqInt yMin, TqInt xMax, TqInt yMax );

    static TqBool CanCull( CqBound* bound )
    {
        //return m_Hierarchy[ 0 ].IsCullable( bound );
		return(TqFalse);
    }
	static CqOcclusionTree& KDTree()
	{
		return(m_KDTree);
	}

protected:
    CqOcclusionBox();
    ~CqOcclusionBox();

    static CqBucket* m_Bucket;
    static CqOcclusionTree	m_KDTree;			///< Tree representing the samples in the bucket.
};



END_NAMESPACE( Aqsis )


#endif // OCCLUSION_H_INCLUDED

