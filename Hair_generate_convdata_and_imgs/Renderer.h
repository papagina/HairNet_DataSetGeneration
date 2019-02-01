//
// Created by zy on 17-1-22.
// Copyright (c) 2016 USC Super Meth Lab All rights reserved.
//
#pragma once



#include <iostream>
#include <stdarg.h>
#include "hair/PiecewiseStrands.h"
#include "hair/XForm.h"

#include <stdio.h>
//#include <windows.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "hair/PiecewiseStrands.h"

#include <fstream>
#include <iostream>

// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <opencv2/opencv.hpp>
#include "hair/XForm.h"

#include <dirent.h>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;

using namespace std;



class Renderer
{
private:

    MyMesh body_mesh;

    string body_mesh_fn="../../body_model/female_halfbody_medium.obj";




    GLfloat* g_vertex_buffer_data;
    GLfloat* g_color_buffer_data;



    bool first_time_bufferdata=true;



public:
    bool _render2Dorient=true;
    bool _add_noise_to_orient =false;
    int hair_vertex_buffer_size = 0;
    int hair_color_buffer_size = 0;

    int body_vertex_buffer_size=0;
    int body_color_buffer_size=0;

    float z_near=0.5;
    float z_far=4.0;

    int width=256;
    int height=256;

    void delete_buffer_data()
    {
        delete g_vertex_buffer_data;
        delete g_color_buffer_data;
    }


    void read_body_mesh()
    {
        if(body_mesh.faces_empty()==true)
        {
            body_mesh.request_vertex_normals();
            OpenMesh::IO::Options read_opt = OpenMesh::IO::Options::VertexNormal;

            if( !OpenMesh::IO::read_mesh(body_mesh, body_mesh_fn, read_opt))
            {
                cout<<"read body mesh error\n";
            }
            else
            {
                cout<<"read body mesh succeeded!\n";

            }


            //cout<<"normals"+to_string(mesh.has_vertex_normals())<<"\n";
            //cout << "face"+mesh.has_face_normals()<<"\n";
        }
    }

    void get_body_vertex_buffer(vector<GLfloat> &vertex_data, vector<GLfloat> &color_data)
    {

        //MyMesh mesh;

        read_body_mesh();

        //Vec3d center(0,0,0);
        double number=0;

        //head_mesh.update_normals();
        //OpenMesh::IO::write_mesh(head_mesh, filename+".obj",OpenMesh::IO::Options::VertexNormal);

        OpenMesh::TriMesh_ArrayKernelT<>::FaceIter f_it(body_mesh.faces_begin()), f_end(body_mesh.faces_end());
        for (; f_it != f_end; ++f_it)
        {
            OpenMesh::TriMesh_ArrayKernelT<>::FaceVertexIter fv_it = body_mesh.fv_iter(f_it.handle());

            glBegin(GL_POLYGON);
            for (; fv_it; ++fv_it)
            {
                vertex_data.push_back(body_mesh.point(fv_it)[0]);
                vertex_data.push_back(body_mesh.point(fv_it)[1]);
                vertex_data.push_back(body_mesh.point(fv_it)[2]);

                //center=center[0]+mesh.point(fv_it)[0];
                //center=center[1]+mesh.point(fv_it)[1];
                //center=center[2]+mesh.point(fv_it)[2];
                number=number+1;
                /*
                normal_data.push_back(mesh.normal(fv_it)[0]);
                normal_data.push_back(mesh.normal(fv_it)[1]);
                normal_data.push_back(mesh.normal(fv_it)[2]);*/
                color_data.push_back(0.0);
                color_data.push_back(0.0);
                color_data.push_back(1.0);

            }
            glEnd();

        }
        //center=center/number;
        //cout<<"Head center"<<center[0]<<" "<<center[1]<<" "<<center[2]<<"\n";

    }



    void get_strand_vertex_buffer(PiecewiseStrand strand, vector<GLfloat> &vertex_data, vector<GLfloat> &color_data, glm::mat4 MVP, bool render_2Dorient=true, Vec3d camera_pos=Vec3d(0,1.6,2))
    {
        Vec3f color(0,0,0.5);

        if (strand.getNumOfPoints() > 1)
        {
            vertex_data.push_back(strand.point(0)[0]);
            vertex_data.push_back(strand.point(0)[1]);
            vertex_data.push_back(strand.point(0)[2]);


            color_data.push_back(color[0]); //B
            color_data.push_back(color[1]);//G
            color_data.push_back(color[2]);//R

            for (int i = 1; i < strand.getNumOfPoints() ; i++) {

                vertex_data.push_back(strand.point(i)[0]);
                vertex_data.push_back(strand.point(i)[1]);
                vertex_data.push_back(strand.point(i)[2]);

                glm::vec4 orient(strand.point(i)[0] - strand.point(i - 1)[0],
                                 strand.point(i)[1] - strand.point(i - 1)[1],
                                 strand.point(i)[2] - strand.point(i - 1)[2], 0);


                glm::vec4 orientproj = MVP * orient;
                //if orient is not (0,0) set color as the orient, else set color as the previous point
                if ((orientproj[0] != 0) || (orientproj[1] != 0)) {
                    float x = orientproj.x;
                    float y = orientproj.y;
                    if (_add_noise_to_orient == true) {
                        //linear_congruential_engine generator;
                        random_device r;
                        default_random_engine generator(r());
                        normal_distribution<float> distribution(0, 0.5);
                        x = distribution(generator) * 0.0015 + x;
                        y = distribution(generator) * 0.0015 + y;
                    }
                    float norm = sqrt(x * x + y * y);

                    if (norm == 0) {
                        color = Vec3f(0, 0, 0);
                    } else {
                        x = x / norm;
                        y = y / norm;
                        if (y < 0)//ambiguous orientation
                        {
                            x = -x;
                            y = -y;
                        }

                        color[0] = x / 2.0 + 0.5;
                        color[1] = y;
                        color[2] = 0.5;

                    }

                }


                if (strand.point_v(i) == 0) {
                    color = Vec3f(0.1, 0.1, 0.1);
                }
                color_data.push_back(color[0]); //B
                color_data.push_back(color[1]);//G
                color_data.push_back(color[2]);//R

                if (i == 1) {
                    color_data[color_data.size() - 6] = color[0];
                    color_data[color_data.size() - 5] = color[1];
                    color_data[color_data.size() - 4] = color[2];
                }

                if (i < strand.getNumOfPoints() - 1) {
                    vertex_data.push_back(strand.point(i)[0]);
                    vertex_data.push_back(strand.point(i)[1]);
                    vertex_data.push_back(strand.point(i)[2]);
                    color_data.push_back(color[0]); //B
                    color_data.push_back(color[1]);//G
                    color_data.push_back(color[2]);//R

                }


            }

        }

    }


    void get_hair_vertex_buffer(PiecewiseStrands hair,vector<GLfloat> &vertex_data, vector<GLfloat> &color_data, glm::mat4 MVP, Vec3d camera_pos)
    {
        vertex_data.clear();
        color_data.clear();
        for (int i = 0; i < hair.getNumOfStrands(); i++)
        {
            vector<GLfloat> vertex_d, color_d;
            get_strand_vertex_buffer(hair.strand(i), vertex_d, color_d, MVP, _render2Dorient, camera_pos);

            /*PiecewiseStrand strand1=hair.strand(i);
            strand1.translateStrand(0.0005,0.0005,0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
            strand1.translateStrand(-0.0005,0.0005,0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
            strand1.translateStrand(0.0005,-0.0005,0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
            strand1.translateStrand(0.0005,0.0005,-0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);

            strand1.translateStrand(-0.0005,-0.0005,0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
            strand1.translateStrand(0.0005,-0.0005,-0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
            strand1.translateStrand(-0.0005,-0.0005,-0.0005);
            get_strand_vertex_buffer(strand1, vertex_d, color_d, MVP, _render2Dorient, camera_pos);
*/
            vertex_data.insert(vertex_data.end(), vertex_d.begin(), vertex_d.end());
            color_data.insert(color_data.end(), color_d.begin(), color_d.end());
        }
    }



    void set_vertex_color_buffer(PiecewiseStrands hair, GLuint &vertexbuffer, GLuint &colorbuffer,  glm::mat4 MVP, Vec3d camera_pos, bool is_render_head=true) {

        // Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
        // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
        vector < GLfloat > hair_vertex_data, hair_color_data, body_vertex_data, body_color_data;
        get_hair_vertex_buffer(hair, hair_vertex_data, hair_color_data, MVP, camera_pos);

        if(is_render_head==true)
            get_body_vertex_buffer(body_vertex_data, body_color_data);

        cout<<"get body vertex buffer done.\n";


        cout<<"get all vertex buffer done.\n";
        hair_vertex_buffer_size = hair_vertex_data.size()/3 ;
        hair_color_buffer_size = hair_color_data.size()/3 ;

        body_vertex_buffer_size = body_vertex_data.size()/3;
        body_color_buffer_size = body_color_data.size()/3;


        vector < GLfloat > vertex_data, color_data;
        vertex_data = hair_vertex_data;
        vertex_data.insert(vertex_data.end(), body_vertex_data.begin(), body_vertex_data.end());

        color_data = hair_color_data;
        color_data.insert(color_data.end(), body_color_data.begin(), body_color_data.end());

        g_vertex_buffer_data = vertex_data.data();
        g_color_buffer_data = color_data.data();




        // This will identify our vertex buffer
        //GLuint vertexbuffer;
        // Generate 1 buffer, put the resulting identifier in vertexbuffer

        // The following commands will talk about our 'vertexbuffer' buffer
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        // Give our vertices to OpenGL.
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), g_vertex_buffer_data, GL_STATIC_DRAW);




        // The following commands will talk about our 'vertexbuffer' buffer
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        // Give our vertices to OpenGL.
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * color_data.size(), g_color_buffer_data, GL_STATIC_DRAW);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );


        // 2rst attribute buffer : vertices
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

        glVertexAttribPointer(
                1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );


    }


    void set_color(GLuint program_id, Vec3d eye_position, PiecewiseStrands hair)
    {
        // One color for each vertex. They were generated randomly.
        Vec3d center = hair.get_hair_center();
        float radius = (float)hair.get_radius(center);

        Vec3d sight_v = center - eye_position;
        glm::vec3 sight_vector = glm::vec3(sight_v[0], sight_v[1], sight_v[2]);
        sight_vector = glm::normalize(sight_vector);
        glm::vec3 hair_center = glm::vec3(center[0], center[1], center[2]);

        GLuint s_v = glGetUniformLocation(program_id, "sight_vec");
        glUniform3fv(s_v, 1, &sight_vector[0]);

        GLuint r = glGetUniformLocation(program_id, "hair_radius");
        glUniform1f(r, radius);

        GLuint h_c = glGetUniformLocation(program_id, "hair_center");
        glUniform3fv(h_c, 1, &hair_center[0]);

    }


//rotation is xzy translation is xyz
    void save_Model_matrix(glm::vec3 translation, glm::vec3 rotation, string fn)
    {
        string out_s="";
        out_s=out_s+to_string(translation[0])+" "+to_string(translation[1])+" "+to_string(translation[2])
              +" " +to_string(rotation[0])+" "+to_string(rotation[1])+" "+to_string(rotation[2]);
        ofstream out(fn);
        out<<out_s;
        out.close();

    }

    void load_Model_matrix(string fn, glm::vec3 &translation, glm::vec3 &rotation)
    {
        ifstream fin(fn);
        float x,y,z;
        fin>>x>>y>>z;
        translation = glm::vec3(x,y,z);

        fin>>x>>z>>y;
        rotation = glm::vec3(x,z,y);

        fin.close();
    }



    //return MVP
    glm::mat4 set_camera(GLuint program_id, glm::vec3 rotation_head=glm::vec3(0,0,0), glm::vec3 translation_head=glm::vec3(0,0,0))
    {

        // Projection matrix : 45�� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(glm::radians(26.5f), (float)(6.0 / 6.0), z_near, z_far);
        if(_render2Dorient==true)
            Projection = glm::perspective(glm::radians(26.5f), (float)(6.0 / 6.0), z_near, z_far);

        // Or, for an ortho camera :
        //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates


        // Camera matrix
        glm::vec3 eye_pos = glm::vec3(0,0,1.3);
        glm::vec3 look_pos=glm::vec3(0,0,-1);
        glm::vec3 head_up = glm::vec3(0,1,0);

        // Camera matrix
        glm::mat4 View = glm::lookAt(
                eye_pos, // Camera is at (4,3,3), in World Space
                look_pos, // and looks at the origin
                head_up // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Model matrix

        glm::mat4 Model =get_model_matrix(rotation_head, translation_head);


        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // Get a handle for our "MVP" uniform
        // Only during the initialisation
        GLuint MatrixID = glGetUniformLocation(program_id, "MVP");

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

        return mvp;
    }

    //return MVP
    glm::mat4 set_camera(GLuint program_id, string model_translation_rotation_matrix)
    {

        // Projection matrix : 45�� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(glm::radians(26.5f), (float)(6.0 / 6.0), z_near, z_far);
        if(_render2Dorient==true)
            Projection = glm::perspective(glm::radians(26.5f), (float)(6.0 / 6.0), z_near, z_far);

        glm::vec3 translation, rotation;

        load_Model_matrix(model_translation_rotation_matrix, translation, rotation);



        glm::vec3 eye_pos = glm::vec3(0,0,1.3);
        glm::vec3 look_pos=glm::vec3(0,0,-1);
        glm::vec3 head_up = glm::vec3(0,1,0);

        // Camera matrix
        glm::mat4 View = glm::lookAt(
                eye_pos, // Camera is at (4,3,3), in World Space
                look_pos, // and looks at the origin
                head_up // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Model matrix

        glm::mat4 Model = get_model_matrix(rotation, translation);


        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // Get a handle for our "MVP" uniform
        // Only during the initialisation
        GLuint MatrixID = glGetUniformLocation(program_id, "MVP");

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

        return mvp;
    }

    glm::mat4 get_model_matrix(glm::vec3 rotation_head=glm::vec3(0,0,0), glm::vec3 translation_head=glm::vec3(0,0,0))
    {
        // Model matrix

        glm::mat4 Model = glm::translate(translation_head) * glm::orientate4(rotation_head) * glm::translate(glm::vec3(0,-1.7,0));
        return Model;
    }

    glm::vec3 get_projected_point(glm::vec3 p)
    {
        int x = round( p[0]/tan(glm::radians(26.5/2.0))/(1.3-p[2])*128+128-1);
        int y = round( p[1]/tan(glm::radians(26.5/2.0))/(1.3-p[2])*128+128-1);
        float z = 1.3-p[2];

        return glm::vec3(x,y,z);
    }

    GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
        std::string VertexShaderCode;
        std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
        if (VertexShaderStream.is_open()) {
            std::string Line = "";
            while (getline(VertexShaderStream, Line))
                VertexShaderCode += "\n" + Line;
            VertexShaderStream.close();
        }
        else {
            printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
            getchar();
            return 0;
        }

        // Read the Fragment Shader code from the file
        std::string FragmentShaderCode;
        std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
        if (FragmentShaderStream.is_open()) {
            std::string Line = "";
            while (getline(FragmentShaderStream, Line))
                FragmentShaderCode += "\n" + Line;
            FragmentShaderStream.close();
        }

        GLint Result = GL_FALSE;
        int InfoLogLength;


        // Compile Vertex Shader
        printf("Compiling shader : %s\n", vertex_file_path);
        char const * VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
        glCompileShader(VertexShaderID);

        // Check Vertex Shader
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }



        // Compile Fragment Shader
        printf("Compiling shader : %s\n", fragment_file_path);
        char const * FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
        glCompileShader(FragmentShaderID);

        // Check Fragment Shader
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0) {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            printf("%s\n", &FragmentShaderErrorMessage[0]);
        }



        // Link the program
        printf("Linking program\n");
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0) {
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }


        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
    }

    void savecurrentDepthImg(string filename, int width, int height)
    {
        float * depth = new float[width*height];
        glReadPixels(0,0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth);

        cv::Mat depth_exr(height, width,CV_32FC3, cv::Scalar(0,0,0));

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                float d = depth[j+i*width];
                float z = 2.0 *z_near * z_far / (z_far + z_near - (d*2-1.0) * (z_far - z_near));
                d=1-(z-0.5);
                d=d*1.5;
                depth_exr.at<cv::Vec3f>(i, j)[0] =d;
                depth_exr.at<cv::Vec3f>(i, j)[1] = d;
                depth_exr.at<cv::Vec3f>(i, j)[2] = d;
            }
        }

        cv::imwrite(filename+".png", (depth_exr)*255);


    }

    void savecurrentDepthImg(string filename, int width, int height, vector<float> hair_convdata)
    {

        cv::Mat depth_exr(height, width,CV_32FC3, cv::Scalar(0,0,0));

        for(int i=0;i<32;i++)
            for(int j=0;j<32;j++) {
                for (int k = 0; k < 100; k++) {
                    float x = hair_convdata[(k * 3 + 0) * 1024 + i * 32 + j];
                    float y = hair_convdata[(k * 3 + 1) * 1024 + i * 32 + j];
                    float z = hair_convdata[(k * 3 + 2) * 1024 + i * 32 + j];

                    if((x==0)&&(y==0)&&(z==0))
                        continue;
                    y=y-1.7;

                    glm::vec3 p = get_projected_point(glm::vec3(x, y, z));

                    if ((p[0] < 255) && (p[0] >= 0) && (p[1] < 255) && (p[1] >= 0)) {
                        float d = 1-(p[2]-0.5);
                        d=d*1.5;
                        if(depth_exr.at<cv::Vec3f>(p[1], p[0])[0]<d ) {
                            depth_exr.at<cv::Vec3f>(p[1], p[0])[0] = d;
                            depth_exr.at<cv::Vec3f>(p[1], p[0])[1] = d;
                            depth_exr.at<cv::Vec3f>(p[1], p[0])[2] = d;
                        }
                    }

                }
            }



        cv::imwrite(filename+".png", (depth_exr)*255);


    }

    void savecurrentImg(string filename, int width, int height)
    {

        float* pixels = new float[3 * width * height];

        glReadPixels(0.0, 0.0, width, height, GL_RGB, GL_FLOAT, pixels);
        cv::Mat renderImg_exr(height, width, CV_32FC3, cv::Scalar(0, 0, 0));
        cv::Mat hairmask_exr(height,width, CV_32FC3, cv::Scalar(0,0,0));
        cv::Mat bodymask_exr(height,width, CV_32FC3, cv::Scalar(0,0,0));

        //cv::Mat renderImg_png(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                float R = pixels[j * 3 + i*width * 3 + 0];
                float G = pixels[j * 3 + i*width * 3 + 1];
                float B = pixels[j * 3 + i*width * 3 + 2];
                renderImg_exr.at<cv::Vec3f>(i, j)[0] = R;
                renderImg_exr.at<cv::Vec3f>(i, j)[1] = G;
                renderImg_exr.at<cv::Vec3f>(i, j)[2] = B;

                if((B<0.6) && (B>0.4)) { //B is around 0.5
                    hairmask_exr.at<cv::Vec3f>(i, j) = cv::Vec3f(1.0, 1.0, 1.0);
                }
                if(B>0.90)
                    bodymask_exr.at<cv::Vec3f>(i,j)=cv::Vec3f(1.0,1.0,1.0);

            }
        }

        cv::Mat renderImgBlur_exr;
        cv::GaussianBlur(renderImg_exr, renderImgBlur_exr,cv::Size(3,3),0,0);
        cv::Mat orientImg_exr = renderImg_exr;//renderImgBlur_exr;

        //cv::Mat renderImg_png(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if(hairmask_exr.at<cv::Vec3f>(i,j)[0]==1.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 1.0;
                }
                else if(bodymask_exr.at<cv::Vec3f>(i,j)[0]==1.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 0.5;
                }
                else
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 0.0;
                }

                if(hairmask_exr.at<cv::Vec3f>(i,j)[0]=0.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j) = cv::Vec3f(0,0,0);
                }
            }
        }


        cv::Mat orientImgFlip_exr;
        cv::flip(orientImg_exr, orientImgFlip_exr,0);
        cv::Mat orientImg255_exr(height, width, CV_32FC3, cv::Scalar(0, 0, 0));
        orientImg255_exr = orientImgFlip_exr*255;
        cv::Mat orientImg_png;
        orientImg255_exr.convertTo(orientImg_png, CV_8UC3);

        cv::imwrite(filename+".exr", orientImgFlip_exr);
        cv::imwrite(filename+".png", orientImg_png);


        delete pixels;
    }

    void savecurrentImg_visable(string filename, int width, int height)
    {
        float* pixels = new float[3 * width * height];

        glReadPixels(0.0, 0.0, width, height, GL_RGB, GL_FLOAT, pixels);
        cv::Mat renderImg_exr(height, width, CV_32FC3, cv::Scalar(0, 0, 0));
        cv::Mat hairmask_exr(height,width, CV_32FC3, cv::Scalar(0,0,0));
        cv::Mat bodymask_exr(height,width, CV_32FC3, cv::Scalar(0,0,0));

        //cv::Mat renderImg_png(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                float R = pixels[j * 3 + i*width * 3 + 0];
                float G = pixels[j * 3 + i*width * 3 + 1];
                float B = pixels[j * 3 + i*width * 3 + 2];
                renderImg_exr.at<cv::Vec3f>(i, j)[0] = R;
                renderImg_exr.at<cv::Vec3f>(i, j)[1] = G;
                renderImg_exr.at<cv::Vec3f>(i, j)[2] = B;

                if((B<0.6) && (B>0.4)) { //B is around 0.5
                    hairmask_exr.at<cv::Vec3f>(i, j) = cv::Vec3f(1.0, 1.0, 1.0);
                }
                if(B>0.90)
                    bodymask_exr.at<cv::Vec3f>(i,j)=cv::Vec3f(1.0,1.0,1.0);

            }
        }

        cv::Mat orientImg_exr = renderImg_exr;//renderImgBlur_exr;

        //cv::Mat renderImg_png(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                if(hairmask_exr.at<cv::Vec3f>(i,j)[0]==1.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 1.0;
                }
                else if(bodymask_exr.at<cv::Vec3f>(i,j)[0]==1.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 0.5;
                }
                else
                {
                    orientImg_exr.at<cv::Vec3f>(i, j)[2] = 0.0;
                }

                if(hairmask_exr.at<cv::Vec3f>(i,j)[0]=0.0)
                {
                    orientImg_exr.at<cv::Vec3f>(i, j) = cv::Vec3f(0,0,0);
                }
            }
        }


        cv::Mat orientImgFlip_exr;
        cv::flip(orientImg_exr, orientImgFlip_exr,0);
        cv::Mat orientImg255_exr(height, width, CV_32FC3, cv::Scalar(0, 0, 0));
        orientImg255_exr = orientImgFlip_exr*255;
        cv::Mat orientImg_png;
        orientImg255_exr.convertTo(orientImg_png, CV_8UC3);

        cv::imwrite(filename, orientImg_png);


        delete pixels;
    }
};
