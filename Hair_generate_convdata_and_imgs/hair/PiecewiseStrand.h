//
//  PiecewiseStrand.h
//
//  Created by Liwen on 10/07/15.
//  Modified by Yi on 06/28/17.
//  Copyright (c)  Liwen and Yi @USC. All rights reserved.
//

#ifndef PIECE_WISE_STRAND_H
#define PIECE_WISE_STRAND_H

#include "XForm.h"
#include "vec.h"
#include <algorithm>    // std::sort


using namespace std;

class PiecewiseStrand
{
public:
	/************ Hair Capture Interface *********/

	int size() const { return m_vecPoint.size(); }

	PiecewiseStrand();
	PiecewiseStrand(const vector<Vec3d> &vecPoint);
	//PiecewiseStrand(const PiecewiseStrand & rhs) {
	//	m_vecPoint = rhs.m_vecPoint;
	//	m_vec
	//}
	virtual ~PiecewiseStrand();

	const Vec3d point(const unsigned int index) const { return m_vecPoint[index]; }
	Vec3d& point(const unsigned int index) { return m_vecPoint[index]; }
	void setPoint(const unsigned int index, const Vec3d& point) { m_vecPoint[index] = point; }

	const bool point_v(const unsigned int index) const { return m_visibility[index]; }
	int& point_v(const unsigned int index) { return m_visibility[index]; }//!!!!!!not sure if it is right
	void setPoint_v(const unsigned int index, const bool v) { m_visibility[index] = v; }

	const vector<Vec3d> vecPoint() const { return m_vecPoint; }
	vector<Vec3d>& vecPoint() { return m_vecPoint; }
	void setVecPoint(const vector<Vec3d> &vecPoint) { m_vecPoint = vecPoint; }

	const vector<int> vecPoint_v() const { return m_visibility; }
	vector<int>& vecPoint_v() { return m_visibility; }
	void setVecPoint_v(const vector<int> &vecPoint_v) { m_visibility = vecPoint_v; }

	void resize(const unsigned int num) { m_vecPoint.resize(num); m_visibility = get_true_list(num); }
	void clear() { m_vecPoint.clear(); m_visibility.clear(); }
	const unsigned int getNumOfPoints() const { return static_cast<unsigned int>(m_vecPoint.size()); }
	const unsigned int getNumOfVisiblePoints() const
	{
		unsigned int n = 0;
		for (int i = 0; i < m_visibility.size(); i++)
		{
			if (m_visibility[i] != 0)
				n++;
		}

		return n;
	}
	void removePoint(const unsigned int index) { m_vecPoint.erase(m_vecPoint.begin() + index); m_visibility.erase(m_visibility.begin() + index); }
	void pushBackPoint(const Vec3d &point) { m_vecPoint.push_back(point); m_visibility.push_back(1); }
	void pushBackPoint(const Vec3d &point, const int &visibility) { m_vecPoint.push_back(point); m_visibility.push_back(visibility); }
    void pushBackCurvature(double c){ m_curvature.push_back(c);}

	bool serializeFloat(FILE *f) const;
	bool serializeFloat_no_visibility_vec(FILE *f) const;
	bool deSerializeFloat(FILE *f);
	bool deSerializeFloat_no_visibility_vec(FILE *f);

	double stranddist(const PiecewiseStrand& strand2) const;
	double stranddist2(const PiecewiseStrand& strand2) const;

	double length() const;
	void scaleStrand(double factor) { for (unsigned int i = 0; i < m_vecPoint.size(); i++) m_vecPoint[i] *= factor; }

	void translateStrand(double x, double y, double z)
	{
		for (unsigned int i = 0; i < m_vecPoint.size(); i++)
			m_vecPoint[i] = m_vecPoint[i]+Vec3d(x,y,z);
	}

	void transformStrand(XForm<double> mat)
	{
		for (unsigned int i = 0; i < m_vecPoint.size(); i++)
			m_vecPoint[i] = mat*m_vecPoint[i];
	}

	void inverse()
	{
		vector<Vec3d> vecPoint(m_vecPoint.size());
		for (unsigned int i = 0; i < m_vecPoint.size(); i++)
		{
			vecPoint[i] = m_vecPoint[m_vecPoint.size() - 1 - i];
		}
		m_vecPoint = vecPoint;

		vector<int> vecPoint_v(m_visibility.size());
		for (unsigned int i = 0; i < m_visibility.size(); i++)
		{
			vecPoint_v[i] = m_visibility[m_vecPoint.size() - 1 - i];
		}
		m_visibility = vecPoint_v;
	}

	PiecewiseStrand lowPassFilter(int halfKernelSize);
	void resampleStrand(const int targetNumOfSamples);



    vector<double> compute_curvature()
    {
        std::vector<double> curv_lst;
        for(int i=0;i<getNumOfPoints();i++)
            curv_lst.push_back(0);


        for(int i=1;i<getNumOfPoints()-1;i++)
        {
            Vec3d lap = m_vecPoint[i]*2.0-m_vecPoint[i-1] - m_vecPoint[i+1];

            double curv = mag(lap);
            curv_lst[i]=curv;
        }

        return curv_lst;
    }



    int find_closest_point_id(Vec3d p)
    {
        int id=0;
        int min_dist=12345;
        for(int i=0;i<m_vecPoint.size();i++)
        {
            double d=mag(p-m_vecPoint[i]);
            if(d<min_dist)
            {
                min_dist=d;
                id=i;
            }
        }
        return  id;
    }


    XForm<double> compute_rotation_between_two_vectors(Vec3d x, Vec3d y)
    {
        double angle = acos(dot(x,y)/(mag(x)*mag(y)));
        Vec3d axis = cross(x,y);
        Vec3d axis2=normalized(axis);
        XForm<double> mat= xform::rot(angle, axis2);
        return mat;
    }

    double get_positive_avg_top_n_max(std::vector<double> v, int num)
    {
        std::sort(v.begin(),v.end());
        double sum=0;
        int n=0;
        for(int i=0;i<num;i++)
        {
            if(v[v.size()-1-i]>0) {
                sum = sum + v[v.size() - 1 - i];
                n++;
            }
        }
        cout<<"\n";
        if(n>0)
            return sum/n;
        else
            return 0;

    }



    vector<double> m_curvature;
protected:
	vector<Vec3d> m_vecPoint;
	vector<int> m_visibility;


private:
	vector<int> get_true_list(int size)
	{
		vector<int> r = vector<int>(size);
		for (int i = 0; i < size; i++)
			r[i] = 1;

		return r;
	}
};

#endif /* defined(PIECE_WISE_STRAND_H) */
