//
//  TrainingDataGenerator.h
//
//  Created by Yi on 06/28/17.
//  Copyright (c) 2017 Yi@USC. All rights reserved.
//

#pragma once
#ifndef TRAININGDATAGENERATOR_H
#define TRAININGDATAGENERATOR_H

//#include <vector>
//#include "hair/PiecewiseStrands.h"
//#include <stdio.h>
#include "HairHelper.h"
#include "hair/cnpy.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "Renderer.h"


using namespace std;

float SCALE = 1.0;//scale the hair 100 times so that the unit is cm
float SAMPLE_PER_STRAND=100;

int ROOTS_MAP_X=32;
int ROOTS_MAP_Z=32;



//write the hair data into .npy
//(STRAND_NUM, POINTS_PER_STRAND_NUM, 4)
//The last dimension is x,y,z,f
//f=1 if growing hair, 0 if stop growing.
//The hair will stop growing when first meet f<FLAG_THRESHOLD


class Hairdata_generator
{
private:
	string intToStrLen5(int i)
	{
		string s;
		char t[256];
        snprintf(t, 10,"%05d", i);
		//sprintf_s(t, "%05d", i);
		s = t;
		return s;
	}



	//std::vector<float> conv_data;

public:


	std::vector<float> read_hairconv_data(string filename)
	{


		//load it into a new array
		cnpy::NpyArray arr = cnpy::npy_load(filename);
		float* loaded_data = arr.data<float>();

        std::vector<float> conv_data;
        for(int i=0;i<ROOTS_MAP_X*ROOTS_MAP_Z*SAMPLE_PER_STRAND*4;i++) {
            conv_data.push_back(loaded_data[i]);

        }

		return conv_data;

	}


	bool write_hairconv(PiecewiseStrands hair, string fn, vector<vector<int>> index_map)
	{
		if(SCALE!=1.0) {
			hair.scaleStrands(SCALE);
		}
        cout<<"transfer hair into convdata.\n";
		std::vector<float> hair_conv = transfer_Hair_into_Hairconvdata(hair, index_map);

        cout<<"write hair convdata.\n";
		write_hairconv(hair_conv, fn);

		return true;
	}

	bool write_hairconv(std::vector<float> hair_conv, string filename)
	{


		cnpy::npy_save(filename,&hair_conv[0],{SAMPLE_PER_STRAND*4, ROOTS_MAP_X,ROOTS_MAP_Z},"w");
		return true;
	}



    //input raw hair with 7937 strands
	std::vector<float> transfer_Hair_into_Hairconvdata(PiecewiseStrands hair,  vector<vector<int>> index_map)
	{


        cout<<"get strand map.\n";
		//**********get downsampled strands
        std::vector<std::vector<PiecewiseStrand>> strand_map;
        std::vector<std::vector<std::vector<double>>> curv_map;
        strand_map.resize(index_map.size());
        curv_map.resize(index_map.size());
        for(int i=0;i<index_map.size();i++)
        {
            strand_map[i].resize(index_map[i].size());
            curv_map[i].resize(index_map[i].size());
            for(int j=0;j<index_map[i].size();j++)
            {
                PiecewiseStrand strand;
                if(index_map[i][j] != -1)
                {
                    strand=hair.strand(index_map[i][j]);
                    strand.resampleStrand(SAMPLE_PER_STRAND);

                }
                else
                {
                    strand.pushBackPoint(Vec3d(0,0,0));
                    strand.resampleStrand(SAMPLE_PER_STRAND);
                }
                strand_map[i][j]=strand;
                curv_map[i][j]=strand.compute_curvature();
            }
        }



        cout<<"resize conv_data_map.\n";
		//********generate hair data
        //(sample_per_strand*4)*map_x*map_z
		std::vector<vector<vector<float>>> conv_data_map;
        conv_data_map.resize(SAMPLE_PER_STRAND*4);
		for(int c=0;c<SAMPLE_PER_STRAND*4;c++)
        {
            conv_data_map[c].resize(ROOTS_MAP_X);
            for(int i=0; i<ROOTS_MAP_X;i++)
            {
                conv_data_map[c][i].resize(ROOTS_MAP_Z);
            }
        }
        cout<<"set conv data map.\n";

        for(int i=0;i<ROOTS_MAP_X;i++)
            for(int j=0;j<ROOTS_MAP_Z;j++)
                for(int k=0;k<SAMPLE_PER_STRAND;k++) {
                    for (int d = 0; d < 3; d++)
                        conv_data_map[k * 4 + d][i][j] = strand_map[i][j].point(k)[d];
                    conv_data_map[k*4+3][i][j]=curv_map[i][j][k];
                }

        cout<<"compute float vector.\n";
        std::vector<float> conv_data;
        for(int c=0;c<SAMPLE_PER_STRAND*4;c++)
        {
            for(int i=0; i<ROOTS_MAP_X;i++)
            {
                for (int j=0;j<ROOTS_MAP_Z;j++)
                    conv_data.push_back(conv_data_map[c][i][j]);
            }
        }
		return conv_data;
	}


	PiecewiseStrands transfer_Hairconvdata_into_Hair(std::vector<float> conv_data)
	{

		PiecewiseStrands hair;

		for(int i=0;i<ROOTS_MAP_X;i++) {
            for (int j = 0; j < ROOTS_MAP_Z; j++) {
                PiecewiseStrand strand;
                for(int p=0;p<SAMPLE_PER_STRAND;p++)
                {
                    double x=conv_data[ (p*4+0)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    double y=conv_data[ (p*4+1)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    double z=conv_data[ (p*4+2)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    double c=conv_data[ (p*4+3)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];

                    Vec3d point(x,y,z);
                    strand.pushBackPoint(point);
                    strand.pushBackCurvature(c);

                }
                hair.pushBackStrand(strand);
            }
        }

        //clean empty strands (0,0,0...)
        PiecewiseStrands hair2;
        for(int i=0;i<hair.getNumOfStrands();i++)
        {
            PiecewiseStrand strand=hair.strand(i);
            bool is_empty=true;

            for(int j=0;j<strand.getNumOfPoints();j++)
            {
                Vec3d point = strand.point(j);
                if((point[0]!=0)||(point[1]!=0)||(point[2]!=0))
                {
                    is_empty=false;
                    break;
                }
            }
            if(is_empty==false)
                hair2.pushBackStrand(strand);
        }

		if(SCALE != 1.0)
			hair2.scaleStrands((1.0/SCALE));

		return hair2;
	}


    PiecewiseStrands transfer_Hairconvdata_into_Hair_with_visibility(std::vector<float> conv_data, vector<float> vis_map)
    {

        PiecewiseStrands hair;

        for(int i=0;i<ROOTS_MAP_X;i++) {
            for (int j = 0; j < ROOTS_MAP_Z; j++) {
                PiecewiseStrand strand;
                for(int p=0;p<SAMPLE_PER_STRAND;p++)
                {
                    double x=conv_data[ (p*4+0)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    double y=conv_data[ (p*4+1)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    double z=conv_data[ (p*4+2)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];

                    float v=vis_map[ (p)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    //float v_y=vis_map[ (p*3+1)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];
                    //float v_z=vis_map[ (p*3+2)*ROOTS_MAP_X*ROOTS_MAP_Z + i*ROOTS_MAP_Z + j ];


                    Vec3d point(x,y,z);
                    strand.pushBackPoint(point,v);


                }
                hair.pushBackStrand(strand);
            }
        }

        //clean empty strands (0,0,0...)
        PiecewiseStrands hair2;
        for(int i=0;i<hair.getNumOfStrands();i++)
        {
            PiecewiseStrand strand=hair.strand(i);
            bool is_empty=true;

            for(int j=0;j<strand.getNumOfPoints();j++)
            {
                Vec3d point = strand.point(j);
                if((point[0]!=0)||(point[1]!=0)||(point[2]!=0))
                {
                    is_empty=false;
                    break;
                }
            }
            if(is_empty==false)
                hair2.pushBackStrand(strand);
        }

        if(SCALE != 1.0)
            hair2.scaleStrands((1.0/SCALE));

        /*cout<<"check visible points.\n";
        for(int i=0;i<hair.getNumOfStrands();i++)
            for(int j=0;j<hair.strand(i).getNumOfPoints();j++)
                if(hair.strand(i).point_v(j)!=0)
                    cout<<"visible point: "<<hair.strand(i).point_v(j);

        cout<<"\n";
*/
        return hair2;
    }

    bool write_hairconv_mask(vector<vector<int>> index_map, string fn)
    {
        std::vector<float> index_lst;
        for(int i=0;i<index_map.size();i++)
        {
            for(int j=0;j<index_map[i].size();j++) {
                if(index_map[i][j]==-1)
                    index_lst.push_back(0);
                else
                    index_lst.push_back(1);

            }
        }

        cnpy::npy_save(fn,&index_lst[0],{1, ROOTS_MAP_X,ROOTS_MAP_Z},"w");
        return true;


    }

    void generate_convdata_mask(string hair_fn, string map_roots_fn, string out_mask_fn) {

        PiecewiseStrands roots;
        roots.deSerializeFloat_no_visibility_vec(map_roots_fn.data());


        vector<vector<int>> roots_id_map;

        PiecewiseStrands hair;
        hair.deSerializeFloat_no_visibility_vec(hair_fn.data());

        //set roots_id_map which is 32*32
        cout<<"roots "<<roots.getNumOfStrands()<<"\n";
        if(roots_id_map.size()==0)
        {
            cout<<"compute roots_id_map.\n";
            roots_id_map.resize(ROOTS_MAP_X);
            for(int i=0;i<ROOTS_MAP_X;i++) {
                roots_id_map[i].resize(ROOTS_MAP_Z);
                for (int j = 0; j < ROOTS_MAP_Z; j++) {
                    Vec3d root = roots.strand(i * ROOTS_MAP_Z + j).point(0);
                    Vec3d normal = roots.strand(i * ROOTS_MAP_Z + j).point(1)-roots.strand(i * ROOTS_MAP_Z + j).point(0);
                    if((root[0]==0)&&(root[1]==0)&&(root[2]==0)&&(normal[0]==0)&&(normal[1]==0)&&(normal[2]==0))
                    {
                        roots_id_map[i][j] = -1;
                    }
                    else
                    {
                        int id = hair.find_closest_strand_by_root(root);

                        roots_id_map[i][j] = id;
                    }
                }
            }
        }

        write_hairconv_mask(roots_id_map, out_mask_fn);
    }


    vector<vector<int>> compute_roots_id_map(PiecewiseStrands roots, PiecewiseStrands hair) {
        vector<vector<int>> roots_id_map;
        cout << "roots " << roots.getNumOfStrands() << "\n";

        cout << "compute roots_id_map.\n";
        roots_id_map.resize(ROOTS_MAP_X);
        for (int i = 0; i < ROOTS_MAP_X; i++) {
            roots_id_map[i].resize(ROOTS_MAP_Z);
            for (int j = 0; j < ROOTS_MAP_Z; j++) {
                Vec3d root = roots.strand(i * ROOTS_MAP_Z + j).point(0);
                Vec3d normal = roots.strand(i * ROOTS_MAP_Z + j).point(1) - roots.strand(i * ROOTS_MAP_Z + j).point(0);
                if ((root[0] == 0) && (root[1] == 0) && (root[2] == 0) && (normal[0] == 0) && (normal[1] == 0) &&
                    (normal[2] == 0)) {
                    roots_id_map[i][j] = -1;
                } else {
                    int id = hair.find_closest_strand_by_root(root);

                    roots_id_map[i][j] = id;
                }
            }
        }


        return roots_id_map;
    }

    //visibility map: 0 means invisible 1 means visible
    //size is (400*32*32)
    vector<float> compute_hairconv_visibility_map(vector<float> hair_convdata, glm::mat4 mvp, glm::mat4 model_matrix)
    {
        Renderer renderer;

        vector<float> visibility_map;
        visibility_map.resize(hair_convdata.size()/4);
        for(int i=0;i<visibility_map.size();i++)
            visibility_map[i]=0;

        float * depth_data = new float[renderer.width*renderer.height];
        glReadPixels(0,0, renderer.width, renderer.height, GL_DEPTH_COMPONENT, GL_FLOAT, depth_data);

        int visible_num=0;
        for(int i=0;i<32;i++)
            for(int j=0;j<32;j++) {
                for (int k = 0; k < 100; k++) {
                    float x = hair_convdata[(k * 4 + 0) * 1024 + i * 32 + j];
                    float y = hair_convdata[(k * 4 + 1) * 1024 + i * 32 + j];
                    float z = hair_convdata[(k * 4 + 2) * 1024 + i * 32 + j];

                    if ((x == 0) && (y == 0) && (z == 0)) {
                        continue;
                    }

                    glm::vec4 p_world=model_matrix*glm::vec4(x,y,z,1.0);

                    glm::vec3 p_project = renderer.get_projected_point(glm::vec3(p_world[0], p_world[1], p_world[2]));

                    if ((p_project[0] < 0) || (p_project[0] > 255) || (p_project[1] < 0) || (p_project[1] > 255))
                        continue;



                    float d = depth_data[int(int(p_project[0]) + int(p_project[1]) * renderer.width)];
                    float depth_z = 2.0 * renderer.z_near * renderer.z_far / (renderer.z_far + renderer.z_near -
                                                                              (d * 2 - 1.0) *
                                                                              (renderer.z_far - renderer.z_near));


                    if ((depth_z+0.05)>p_project[2]) {
                        visibility_map[(k ) * 1024 + i * 32 + j] = 1;
                        //visibility_map[(k * 3 + 1) * 1024 + i * 32 + j] = 1;
                        //visibility_map[(k * 3 + 2) * 1024 + i * 32 + j] = 1;
                        visible_num++;
                    }

                }

            }


        cout<<"visible num: "<<visible_num<<"\n";

        return visibility_map;
    }


    bool write_hairconv_visibility_map(vector<float> hair_convdata, glm::mat4 mvp, glm::mat4 model_matrix, string out_fn)
    {

        vector<float> visibility_map=compute_hairconv_visibility_map(hair_convdata, mvp,model_matrix);

        cnpy::npy_save(out_fn,&visibility_map[0],{SAMPLE_PER_STRAND, ROOTS_MAP_X,ROOTS_MAP_Z},"w");

    }

    std::vector<float> read_hairconv_visibility_map(string filename)
    {


        //load it into a new array
        cnpy::NpyArray arr = cnpy::npy_load(filename);
        float* loaded_data = arr.data<float>();

        std::vector<float> vis_data;
        for(int i=0;i<ROOTS_MAP_X*ROOTS_MAP_Z*SAMPLE_PER_STRAND;i++) {
            vis_data.push_back(loaded_data[i]);

        }

        return vis_data;

    }



};


#endif
