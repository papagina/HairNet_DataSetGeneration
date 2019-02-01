//
//  PiecewiseStrand.cpp
//
//  Created by Liwen on 10/07/15.
//  Modified by Yi on 06/28/17.
//  Copyright (c)  Liwen and Yi @USC. All rights reserved.
//

#include "PiecewiseStrand.h"

PiecewiseStrand::PiecewiseStrand()
{

}



PiecewiseStrand::PiecewiseStrand(const vector<Vec3d>& vecPoint)
	: m_vecPoint(vecPoint)
{
	m_visibility = get_true_list(vecPoint.size());
	
}

PiecewiseStrand::~PiecewiseStrand()
{

}

bool PiecewiseStrand::serializeFloat(FILE *f) const
{
	int num = (int)m_vecPoint.size();
	fwrite(&num, sizeof(int), 1, f);
	for (unsigned int i = 0; i < m_vecPoint.size(); i++)
	{
		float x = (float)m_vecPoint[i][0];
		float y = (float)m_vecPoint[i][1];
		float z = (float)m_vecPoint[i][2];
		int v = (int)m_visibility[i];
		fwrite(&x, sizeof(float), 1, f);
		fwrite(&y, sizeof(float), 1, f);
		fwrite(&z, sizeof(float), 1, f);
		fwrite(&v, sizeof(int), 1, f);
	}
	return true;
}

bool PiecewiseStrand::serializeFloat_no_visibility_vec(FILE *f) const
{
	int num = (int)m_vecPoint.size();
	fwrite(&num, sizeof(int), 1, f);
	for (unsigned int i = 0; i < m_vecPoint.size(); i++)
	{
		float x = (float)m_vecPoint[i][0];
		float y = (float)m_vecPoint[i][1];
		float z = (float)m_vecPoint[i][2];
		
		fwrite(&x, sizeof(float), 1, f);
		fwrite(&y, sizeof(float), 1, f);
		fwrite(&z, sizeof(float), 1, f);
	}
	return true;
}

bool PiecewiseStrand::deSerializeFloat(FILE *f)
{
	int numOfPoints;
	fread(&numOfPoints, sizeof(int), 1, f);
	m_vecPoint.resize(numOfPoints);
	m_visibility.resize(numOfPoints);
	for (int i = 0; i < numOfPoints; i++)
	{
		float x, y, z;
		int v;
		fread(&x, sizeof(float), 1, f);
		fread(&y, sizeof(float), 1, f);
		fread(&z, sizeof(float), 1, f);
		fread(&v, sizeof(int), 1, f);
		m_vecPoint[i] = Vec3d(x, y, z);
		m_visibility[i] =  v;
	}
	return true;
}

bool PiecewiseStrand::deSerializeFloat_no_visibility_vec(FILE *f)
{
	int numOfPoints;
	fread(&numOfPoints, sizeof(int), 1, f);
    //cout<<"num of points: "<<numOfPoints<<"\n";
    if ( ((numOfPoints>=0)==false) || ((numOfPoints<1000)==false) )
        return false;
	m_vecPoint.resize(numOfPoints);
	m_visibility.resize(numOfPoints);
	for (int i = 0; i < numOfPoints; i++)
	{
		float x, y, z;
		fread(&x, sizeof(float), 1, f);
		fread(&y, sizeof(float), 1, f);
		fread(&z, sizeof(float), 1, f);
        //cout<<x<<" "<<y<<" "<<z<<" ";
		m_vecPoint[i] = Vec3d(x, y, z);
		m_visibility[i] = 1;
	}
    //cout<<"\n";
	return true;
}


double PiecewiseStrand::stranddist(const PiecewiseStrand& strand2) const
{
	double distMinMax = -1e15;
	double distMinSum = 0.0;
	for (unsigned int i = 0; i < getNumOfPoints(); i++)
	{
		double distMin = 1e15;
		int idxMin = -1;
		for (unsigned int j = 0; j < strand2.getNumOfPoints(); j++)
		{
			double distTmp = mag(point(i) - strand2.point(j));
			if (distTmp < distMin)
			{
				distMin = distTmp;
				idxMin = j;
			}
		}
		distMinMax = max(distMinMax, distMin);
		distMinSum = distMinSum + distMin;
	}
	return distMinSum / (double)getNumOfPoints();
}

double PiecewiseStrand::stranddist2(const PiecewiseStrand& strand2) const
{
	double distMinMax = -1e15;
	double distMinSum = 0.0;
	for (unsigned int i = 0; i < getNumOfPoints(); i++)
	{
		double distMin = 1e15;
		int idxMin = -1;
		for (unsigned int j = 0; j < strand2.getNumOfPoints(); j++)
		{
			double distTmp = mag2(point(i) - strand2.point(j));
			if (distTmp < distMin)
			{
				distMin = distTmp;
				idxMin = j;
			}
		}
		distMinMax = max(distMinMax, distMin);
		distMinSum = distMinSum + distMin;
	}
	return distMinSum / (double)getNumOfPoints();
}

double PiecewiseStrand::length() const
{
	double l = 0.0;
	for (int i = 0; i < getNumOfPoints() - 1; i++)
	{
		l += mag(point(i + 1) - point(i));
	}
	return l;
}

double* ComputeGaussianKernel(const int inRadius, const float inWeight)
{
	int mem_amount = (inRadius * 2) + 1;
	double* gaussian_kernel = (double*)malloc(mem_amount * sizeof(double));

	double twoRadiusSquaredRecip = 1.0 / (2.0 * inRadius * inRadius);
	double sqrtTwoPiTimesRadiusRecip = 1.0 / (sqrt(2.0 * 3.1415926585) * inRadius);
	double radiusModifier = inWeight;

	// Create Gaussian Kernel
	int r = -inRadius;
	double sum = 0.0f;
	for (int i = 0; i < mem_amount; i++)
	{
		double x = r * radiusModifier;
		x *= x;
		double v = sqrtTwoPiTimesRadiusRecip * exp(-x * twoRadiusSquaredRecip);
		gaussian_kernel[i] = v;

		sum += v;
		r++;
	}

	// Normalize distribution
	double div = sum;
	for (int i = 0; i < mem_amount; i++)
		gaussian_kernel[i] /= div;

	return gaussian_kernel;
}

PiecewiseStrand PiecewiseStrand::lowPassFilter(int halfKernelSize) {


    PiecewiseStrand strand;
    for(int i=0;i<getNumOfPoints();i++)
        strand.pushBackPoint(point(i));

    if (halfKernelSize <= 0) {
        cout<<"Half kernel size is 0!\n";

        return strand;
    }
    if (getNumOfPoints() <=3) {

        return strand;
    }


    double* kernal=ComputeGaussianKernel(halfKernelSize, 1.0);
    /*cout<<"kernal: ";
    for(int i=0;i<halfKernelSize*2+1;i++)
        cout<<kernal[i]<<" ";
    cout<<"\n";
*/

    for(int i=0;i<getNumOfPoints();i++) {
        int left = i - halfKernelSize;
        int right = i + halfKernelSize;
        double weight_sum = 0;
        Vec3d point_sum = Vec3d(0, 0, 0);
        int n=0;
        for (int j = left; j <= right;j++) {
            if ((j < 0) || (j >= getNumOfPoints()))
                continue;
            else {
                point_sum = point_sum + point(j)*kernal[n];
                weight_sum = weight_sum + kernal[n];
            }
            n++;
        }

        if(weight_sum == 0)
            strand.point(i) = point(i);
        else
            strand.point(i)=point_sum/weight_sum;

    }
    return strand;

}

void PiecewiseStrand::resampleStrand(const int targetNumOfSamples)
{
	int numOfSamples = getNumOfPoints();

	if ( numOfSamples <= 0)
	{
		return;
	}

	vector<Vec3d> vecSamples;
	for (int i = 0; i < targetNumOfSamples; i++)
	{
		double fi = i * (numOfSamples - 1) / double(targetNumOfSamples - 1);
		int idx0 = int(fi);
		int idx1 = idx0 + 1;
		idx1 = (idx1 >= numOfSamples) ? (numOfSamples - 1) : idx1;
		double wt1 = fi - idx0;
		double wt0 = 1.0 - wt1;
		Vec3d pos0 = point(idx0);
		Vec3d pos1 = point(idx1);
		Vec3d pos = pos0 * wt0 + pos1 * wt1;
		vecSamples.push_back(pos);
	}
	m_vecPoint = vecSamples;

	m_visibility = get_true_list(m_vecPoint.size());

}

