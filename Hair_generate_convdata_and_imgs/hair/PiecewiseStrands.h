//
//  PiecewiseStrand.h
//
//  Created by Liwen on 10/07/15.
//  Modified by Yi on 06/28/17.
//  Copyright (c)  Liwen and Yi @USC. All rights reserved.
//

#ifndef PIECE_WISE_STRANDS_H
#define PIECE_WISE_STRANDS_H

#include "PiecewiseStrand.h"
#include <iomanip>
#include <sstream>

using namespace std;

class PiecewiseStrands
{
public:
	PiecewiseStrands();
	PiecewiseStrands(const vector<PiecewiseStrand> &vecStrand);
	virtual ~PiecewiseStrands();

	const PiecewiseStrand strand(const unsigned int index) const { return m_vecStrand[index]; }
	PiecewiseStrand& strand(const unsigned int index) { return m_vecStrand[index]; }
	void setStrand(const unsigned int index, const PiecewiseStrand& strand) { m_vecStrand[index] = strand; }


	const vector<PiecewiseStrand> vecStrand() const { return m_vecStrand; }
	vector<PiecewiseStrand>& vecStrand() { return m_vecStrand; }
	void setVecStrand(const vector<PiecewiseStrand> &vecStrand) { m_vecStrand = vecStrand; }

	void resize(const unsigned int num) { m_vecStrand.resize(num); }
	void clear() { m_vecStrand.clear(); }
	const unsigned int getNumOfStrands() const { return static_cast<unsigned int>(m_vecStrand.size()); }
	void removeStrand(const unsigned int index) { m_vecStrand.erase(m_vecStrand.begin() + index); }
	void pushBackStrand(const PiecewiseStrand &strand) { m_vecStrand.push_back(strand); }

	bool serialize(const char* fileName) const;
	bool serializeFloat_no_visibility_vec(const char* fileName) const;
	bool deSerialize(FILE *f);
	bool deSerialize(const char* fileName);
	bool deSerializeFloat_no_visibility_vec(FILE *f);
	bool deSerializeFloat_no_visibility_vec(const char* fileName);

	bool dumpStrandsAsBinaryData(string fileName);

	double stranddist(const PiecewiseStrands& strands2) const;
	double stranddist2(const PiecewiseStrands& strands2) const;

	void scaleStrands(double factor) { for (unsigned int i = 0; i < m_vecStrand.size(); i++) m_vecStrand[i].scaleStrand(factor); }

    int find_closest_strand_by_root(Vec3d p)
    {
        double min_dist=123456;
        int min_id=0;
        for(int i=0;i<getNumOfStrands();i++)
        {
            Vec3d root = strand(i).point(0);
            double dist = mag((root-p));
            if(dist<min_dist)
            {
                min_dist=dist;
                min_id=i;
            }
        }
        return min_id;
    }

    void translateStrands(double x, double y, double z)
    {
        for (unsigned int i = 0; i < m_vecStrand.size(); i++)
            m_vecStrand[i].translateStrand(x,y,z);
    }

	void transformStrands(XForm<double> mat)
	{
		for (unsigned int i = 0; i < m_vecStrand.size(); i++)
			m_vecStrand[i].transformStrand(mat);
	}

	void inverseStrands() { for (unsigned int i = 0; i < m_vecStrand.size(); i++) m_vecStrand[i].inverse(); }

	//return an array of 3d position for each point in the Global index
	vector<Vec3d> get_point_cloud()
	{
		vector<Vec3d> pc;
		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			PiecewiseStrand strand = m_vecStrand[i];
			for (int j = 0; j< strand.getNumOfPoints(); j++)
			{
				pc.push_back(strand.point(j));
			}
		}
		return pc;
	}

	//return an array of [strandID, pointID] for each point in the Global index.
	vector<Vec2i> get_point_index_on_hair()
	{
		vector<Vec2i> pc_index;
		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			PiecewiseStrand strand = m_vecStrand[i];
			for (int j = 0; j< strand.getNumOfPoints(); j++)
			{
				Vec2i index;
				index[0] = i;
				index[1] = j;
				pc_index.push_back(index);
			}
		}
		return pc_index;
	}

	Vec3d get_avg_point()
	{
		Vec3d avg_point(0, 0, 0);
		int point_num = 0;
		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			PiecewiseStrand strand = m_vecStrand[i];
			for (int j = 0; j< strand.getNumOfPoints(); j++)
			{
				avg_point = avg_point + strand.point(j);
				point_num = point_num + 1;
			}
		}
		avg_point = avg_point / (float)point_num;
		return avg_point;
	}

	//set all visiblity to 1
	void reset_visiblity()
	{

		for (int i = 0; i < m_vecStrand.size(); i++)
		{

			for (int j = 0; j< m_vecStrand[i].getNumOfPoints(); j++)
			{
				m_vecStrand[i].point_v(j) = 1;
			}
		}

	}



	//intervel is mm
	PiecewiseStrands get_hairSegments_with_certain_segment_len(float segment_len)
	{
		int strandNum = m_vecStrand.size();
		PiecewiseStrands newStrands;
		for (int i = 0; i < strandNum; i++)
		{
			PiecewiseStrand strand = m_vecStrand[i];
			PiecewiseStrand newstrand = strand;
			double strandlength = strand.length();
			int pointNum = strandlength / segment_len;
            newstrand.resampleStrand(pointNum);
			newStrands.pushBackStrand(newstrand);
		}
		return newStrands;
	}

	int get_point_num()
	{
		int point_num = 0;
		for (int i = 0; i < m_vecStrand.size(); i++)
			point_num = point_num + m_vecStrand[i].getNumOfPoints();

		return point_num;
	}

	Vec3d get_hair_center()
	{
		Vec3d center = Vec3d(0, 0, 0);
		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			for (int j = 0; j < m_vecStrand[i].getNumOfPoints(); j++)
			{
				center = center + m_vecStrand[i].point(j);
			}
		}

		double point_num = (double)get_point_num();
		center = center *(1.0 / point_num);
		return center;
	}

	Vec3d get_roots_center()
	{
		Vec3d center = Vec3d(0, 0, 0);

		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			center = center + m_vecStrand[i].point(0);
		}

		double point_num = m_vecStrand.size();
		center = center *(1.0 / point_num);
		return center;
	}

	double get_radius(Vec3d center)
	{

		double max_dist = 0;
		for (int i = 0; i < m_vecStrand.size(); i++)
		{
			for (int j = 0; j < m_vecStrand[i].getNumOfPoints(); j++)
			{
				Vec3d dist_vec = m_vecStrand[i].point(j) - center;
				double dist = dist_vec[0] * dist_vec[0] + dist_vec[1] * dist_vec[1] + dist_vec[2] * dist_vec[2];
				dist = sqrt(dist);
				if (max_dist < dist)
					max_dist = dist;
			}
		}
		return max_dist;
	}

	double get_radius()
	{
		Vec3d center = get_hair_center();
		return get_radius(center);
	}


    int get_max_strand_point_num()
    {
        int max_len=-12345;
        for(int i=0;i<m_vecStrand.size();i++)
        {
            int length = m_vecStrand[i].getNumOfPoints();
            if(max_len<length)
                max_len=length;
        }
        return max_len;
    }

	float get_max_strand_len(int sample_rate=1)
	{
		float max_len=-12345;
		for(int i=0;i<m_vecStrand.size();i=i+sample_rate)
		{
			float length = m_vecStrand[i].length();
			if(max_len<length)
				max_len=length;
		}
		return max_len;
	}



	double get_random_delta_parameters(double l)
	{
		double x = 0.5*sin(100*l)+0.2*sin(110*l)+0.3*sin(150*l);
		return x;
	}



	PiecewiseStrands get_low_pass_smooth_hair(double seg_len, int half_kernal_size)
	{
		PiecewiseStrands hair;
		for(int i=0;i<getNumOfStrands();i++)
		{

			PiecewiseStrand s = strand(i);
            int original_point_num=s.getNumOfPoints();
			double len=s.length();

			int sample_num = len/seg_len;
			if(sample_num==0)
				sample_num=0;

			s.resampleStrand(sample_num);

			PiecewiseStrand smooth_s = s.lowPassFilter(half_kernal_size);
            smooth_s.resampleStrand(original_point_num);
			hair.pushBackStrand(smooth_s);
		}
		return hair;
	}



private:
	vector<PiecewiseStrand> m_vecStrand;
};

#endif /* defined(PIECE_WISE_STRANDS_H) */
