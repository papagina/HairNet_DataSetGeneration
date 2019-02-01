//
//  PiecewiseStrand.cpp
//
//  Created by Liwen on 10/07/15.
//  Modified by Yi on 06/28/17.
//  Copyright (c)  Liwen and Yi @USC. All rights reserved.
//

#include "PiecewiseStrands.h"

PiecewiseStrands::PiecewiseStrands()
{
    
}

PiecewiseStrands::PiecewiseStrands(const vector<PiecewiseStrand> &vecStrand)
: m_vecStrand(vecStrand)
{
}

PiecewiseStrands::~PiecewiseStrands()
{
    
}

bool PiecewiseStrands::serialize(const char* fileName) const
{
    FILE* f = fopen(fileName, "wb");
    if ( !f )
	{
		cout<<"Failed to write models to binary file"<<endl;
		return false;
	}
    
    int num = (int)m_vecStrand.size();
    fwrite(&num, sizeof(int), 1, f);
    
    for (unsigned int i=0; i < m_vecStrand.size(); i++) {
        m_vecStrand[i].serializeFloat(f);
    }
    fclose(f);
    return true;
}

bool PiecewiseStrands::serializeFloat_no_visibility_vec(const char* fileName) const
{
	FILE* f = fopen(fileName, "wb");
	if (!f)
	{
		cout << "Failed to write models to binary file" << endl;
		return false;
	}

	int num = (int)m_vecStrand.size();
	fwrite(&num, sizeof(int), 1, f);

	for (unsigned int i = 0; i < m_vecStrand.size(); i++) {
		m_vecStrand[i].serializeFloat_no_visibility_vec(f);
	}
	fclose(f);
	return true;
}
	
bool PiecewiseStrands::deSerialize(const char* fileName)
{
	FILE *f = fopen(fileName, "rb");
	return deSerialize(f);
}

bool PiecewiseStrands::deSerialize(FILE *f)
{
	if (!f)
	{
		cout << "Failed to read models from binary file" << endl;
		return false;
	}

	int num;
	fread(&num, sizeof(int), 1, f);

	int point_num = 0;
	//int length = 0;
	for (int i = 0; i < num; i++)
	{
		PiecewiseStrand strand;
		strand.deSerializeFloat(f);
		m_vecStrand.push_back(strand);
		point_num = point_num + strand.getNumOfPoints();
		//length = length + strand.length();
	}

	//cout << "Load " << num << " strands, " << point_num << " points, " << endl;
	fclose(f);
	return true;
}

bool PiecewiseStrands::deSerializeFloat_no_visibility_vec(FILE *f)
{
	
	if (!f)
	{
		cout << "Failed to read models from binary file" << endl;
		return false;
	}

	int num;
	fread(&num, sizeof(int), 1, f);
	cout << "Strand number: " << num << "\n";
	//int point_num = 0;
	//int length = 0;
	for (int i = 0; i < num; i++)
	{
		PiecewiseStrand strand;
        bool s= strand.deSerializeFloat_no_visibility_vec(f);
        if(s==false)
        {
            cout << "Failed to read models from binary file. Wrong strand point number" << endl;
            return false;
        }
        m_vecStrand.push_back(strand);

	}

	//cout << "Load " << num << " strands, " << point_num << " points, " << endl;
	//fclose(f);
	return true;
}


bool PiecewiseStrands::deSerializeFloat_no_visibility_vec(const char* fileName)
{
	FILE *f = fopen(fileName, "rb");
	bool result= deSerializeFloat_no_visibility_vec(f);
	fclose(f);
	return result;
}



bool PiecewiseStrands::dumpStrandsAsBinaryData(string fileName)
{
	FILE* f = fopen(fileName.c_str(), "wb");
	int a = 1;
	fwrite(&a, 4, 1, f);
	int nstrands = (int)m_vecStrand.size();
	fwrite(&nstrands, 4, 1, f);
	for ( int i=0; i<nstrands; i++ )
    {
		int nverts = strand(i).getNumOfPoints();
	    fwrite(&nverts, 4, 1, f);
	    for ( int j=0; j<nverts; j++ )
        {
            Vec3d p = strand(i).point(j);
            Vec3d pj = Vec3d((float)p[0], (float)p[1], (float)p[2]);
            fwrite(&pj[0], 4, 1, f);
			fwrite(&pj[1], 4, 1, f);
			fwrite(&pj[2], 4, 1, f);
	    }
	}
    fclose(f);
    return true;
}



double PiecewiseStrands::stranddist(const PiecewiseStrands& strands2) const
{
	double distMinSum = 0.0;
	for (unsigned int i = 0; i < getNumOfStrands(); i++)
	{
		distMinSum += strand(i).stranddist(strands2.strand(i));
	}
    return distMinSum;
}

double PiecewiseStrands::stranddist2(const PiecewiseStrands& strands2) const
{
	double distMinSum = 0.0;
	for (unsigned int i = 0; i < getNumOfStrands(); i++)
	{
		distMinSum += strand(i).stranddist2(strands2.strand(i));
	}
    return distMinSum;
}
