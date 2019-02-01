//
// Created by yi on 9/14/17.
//  Copyright (c) 2017 Yi@USC. All rights reserved.
//

#ifndef HAIRVIEWER3_HAIRHELPER_H
#define HAIRVIEWER3_HAIRHELPER_H

#endif //HAIRVIEWER3_HAIRHELPER_H

#include <vector>
#include "hair/PiecewiseStrands.h"
#include <stdio.h>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>

typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;

using namespace std;



class HairHelper {

public:

    double get_norm(Vec3d v)
    {
        double n = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
        n=sqrt(n);
        return n;
    }
    double get_closest_distance(vector <Vec3d> point_list, Vec3d point) {
        double min_dis = 10000000000;
        for (int i = 0; i < point_list.size(); i++) {
            double dis = get_norm(point_list[i] - point);
            if (dis < min_dis) {
                min_dis = dis;
            }
        }
        return min_dis;
    }

    int get_farest_root_id(vector <Vec3d> root_list, PiecewiseStrands strands) {
        double max_dis = 0;
        int id = 0;
        for (int i = 0; i < strands.getNumOfStrands(); i++) {
            if(strands.strand(i).getNumOfPoints()>1) {
                Vec3d root_dir = strands.strand(i).point(0) - Vec3d(-0.001546, 1.738949, -0.000979) ;//strands.strand(i).point(1);
                normalize(root_dir);

                double dis = get_closest_distance(root_list, root_dir);
                if (dis > max_dis) {
                    max_dis = dis;
                    id = i;
                }
            }
        }
        return id;
    }



    //the result may have sampleNum+-1 strands
    PiecewiseStrands get_hair_with_certain_strand_number(PiecewiseStrands strands, int strand_num)
    {
        PiecewiseStrands newStrands;
        int sampleInterval = strands.getNumOfStrands() / strand_num;
        int strandNum = strands.getNumOfStrands();
        for (int i = 0; i < strandNum; i = i + sampleInterval)
        {
            newStrands.pushBackStrand(strands.strand(i));
        }
        return newStrands;
    }


    //input a root, output the strand whose root is closest to root, and the strand's length should be bigger than 0.01
    PiecewiseStrand get_nearest_good_strand(PiecewiseStrands *hair, Vec3d root)
    {
        int id=-1;
        double min_dist=123456;
        for (int i = 0; i < hair->getNumOfStrands(); ++i)
        {
            double dist= get_norm( hair->strand(i).point(0)-root);
            if(dist<min_dist)
            {
                if(hair->strand(i).length()>=0.01)
                {
                    min_dist=dist;
                    id=i;
                }
            }
        }

        return hair->strand(id);
    }

    PiecewiseStrand get_translated_strand(PiecewiseStrand strand, Vec3d vector)
    {
        for(int i=0;i<strand.size();i++)
            strand.point(i)=strand.point(i)+vector;
        return strand;
    }
    //the result may have sampleNum+-1 strands
    PiecewiseStrands get_hair_by_index_lst(PiecewiseStrands *strands, vector<int> index_lst)
    {
        PiecewiseStrands newStrands;
        int strandNum=index_lst.size();
        for (int i = 0; i < strandNum;i++)
        {
            PiecewiseStrand sample_strand=strands->strand(index_lst[i]);

            /*
            if(sample_strand.length()<0.015)
            {// find a nearest good strand
                sample_strand=get_nearest_good_strand(strands, strands->strand(index_lst[i]).point(0));
                //move the strand to the root position.
                Vec3d T=strands->strand(index_lst[i]).point(0)- sample_strand.point(0);
                sample_strand = get_translated_strand(sample_strand,T);
            }*/
            newStrands.pushBackStrand(sample_strand);
        }
        return newStrands;
    }

    bool write_vector_to_file(const char* fileName, vector<int> lst)
    {

        FILE* f = fopen(fileName, "wb");
        if ( !f )
        {
            cout<<"Failed to write models to binary file"<<endl;
            return false;
        }

        int num = (int)lst.size();
        cout<<"current index lst number: "<<to_string(num)<<"\n";
        fwrite(&num, sizeof(int), 1, f);

        for (unsigned int i = 0; i < lst.size(); i++)
        {

            fwrite(&lst[i], sizeof(int), 1, f);
        }
        fclose(f);
        return true;

    }

    vector<int> read_vector_from_file(const char* fileName)
    {
        vector<int> lst;
        FILE* f = fopen(fileName, "rb");
        if ( !f )
        {
            cout<<"Failed to write models to binary file"<<endl;
            return lst;
        }

        int num ;
        fread(&num, sizeof(int), 1, f);


        for (unsigned int i = 0; i < num; i++)
        {
            int x;
            fread(&x, sizeof(int), 1, f);
            lst.push_back(x);
        }
        fclose(f);
        return lst;
    }



};