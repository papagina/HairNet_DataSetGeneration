//
//  main.cpp
//
//  Created by Yi on 06/28/17.
//  Copyright (c) 2017 Yi@USC. All rights reserved.
//



#include <iostream>
#include <stdarg.h>
#include "hair/PiecewiseStrands.h"
//#include "hair/XForm.h"
#include "TrainingDataGenerator.h"
#include <stdio.h>

#include "Renderer.h"
#include <GLFW/glfw3.h>
#include <thread>

#include <dirent.h>
#include <sys/stat.h>

using namespace std;


string vertexshader_fn ="../SimpleVertexShader.vertexshader";
string fragmentshader_fn = "../SimpleFragmentShader.fragmentshader";


string intToStrLen5(int i)
{
    string s;
    char t[256];
    snprintf(t, 10, "%05d",i);
    //sprintf_s(t, "%05d", i);
    s = t;
    return s;
}





void generate_trainingData(string hair_folder, string convdata_folder, string map_roots_fn)
{



    PiecewiseStrands roots;
    roots.deSerializeFloat_no_visibility_vec(map_roots_fn.data());


    vector<vector<int>> roots_id_map;

    Hairdata_generator hairdata_generator;


    DIR           *dirp;
    struct dirent *directory;

    dirp = opendir(hair_folder.data());
    if (!dirp) {
        cout<<"no such hair folder.\n";
        return;
    }
    int num=0;
    while ((directory = readdir(dirp)) != NULL) {

        string hair_fn = hair_folder + directory->d_name;
        string hair_name0 = directory->d_name;
        if(hair_name0.size()<5)
            continue;
        if(hair_name0.substr(hair_name0.size()-5, hair_name0.size())!=".data")
            continue;
        string hair_name = hair_name0.substr(0, hair_name0.size() - 5);

        string convdata_fn = convdata_folder + hair_name + ".convdata";


        PiecewiseStrands hair;


        cout<<"####hair "<<to_string(num)<<"\n";
        cout << "Generate " + convdata_fn + "\n";


        hair.deSerializeFloat_no_visibility_vec(hair_fn.data());


        //set roots_id_map which is 32*32
        if(roots_id_map.size()<1)
            roots_id_map = hairdata_generator.compute_roots_id_map(roots,hair);


        cout<<"Generate hair conv data.\n";
        hairdata_generator.write_hairconv(hair, convdata_fn, roots_id_map);
        num=num+1;

    }
}




float RandomFloat(float min, float max)
{

    // this  function assumes max > min, you may want
    // more robust error checking for a non-debug build
    assert(max > min);
    float random = ((float) rand()) / (float) RAND_MAX;

    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    float range = max - min;
    return (random*range) + min;
}


void visualize_and_save_hair_orient(string hair_folder, string hair_convdata_folder, string orient_folder, int view_num)
{
    srand(time(0));

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


    GLFWwindow* window = glfwCreateWindow(256, 256, "OpenGL", nullptr, nullptr); // Windowed


    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    //VAO
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    // Create and compile our GLSL program from the shaders
    Renderer renderer;
    renderer._render2Dorient=true;

    renderer._add_noise_to_orient=true;
    GLuint programID = renderer.LoadShaders(vertexshader_fn.data(),fragmentshader_fn.data());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Use our shader
    glUseProgram(programID);

    int hair_num=0;

    DIR           *dirp;
    struct dirent *directory;

    dirp = opendir(hair_folder.data());
    if (!dirp) {
        cout<<"no such hair folder.\n";
        return;

    }
    GLuint vertexbuffer, colorbuffer;
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &colorbuffer);



    int sum_bad_hair=0;
    while ((directory = readdir(dirp)) != NULL) {
        string d_name = directory->d_name;


        if(d_name.length()<5)
            continue;

        if(d_name.substr(d_name.size()-5, d_name.size())!=".data")
            continue;

        string hair_fn = hair_folder + d_name;
        string hair_name = d_name.substr(0, d_name.size() - 5);
        string hair_convdata_fn = hair_convdata_folder+hair_name+".convdata";

        hair_num++;
        cout<<"#######Process strand "<<to_string(hair_num)<<"\n";
        cout << "hair_name: " + hair_name << "\n";


        string o_fn=orient_folder + hair_name + "_v" + to_string(view_num-1)+".exr";
        ifstream ifile(o_fn);

        if (ifile) {
            // The orientation images have already been generated.
            cout<<"The hair has already been rendered\n";
            continue;
        }

        Hairdata_generator hg;
        cout<<"load "<<hair_convdata_fn<<"\n";
        vector<float> hair_convdata=hg.read_hairconv_data(hair_convdata_fn);


        PiecewiseStrands hair_raw;

        cout<<"load "<<hair_fn<<"\n";

        bool s=hair_raw.deSerializeFloat_no_visibility_vec(hair_fn.data());

        if(s==false)
        {
            sum_bad_hair++;
            cout<<"sum bad hair: "<<sum_bad_hair<<"\n";
            continue;
        }

        cout<<"load succeed.\n";

        PiecewiseStrands hair=hair_raw;


        float hair_max_strand_len = hair.get_max_strand_len(10);

        cout<<"Max strand len: "<<hair_max_strand_len<<"\n";
        //Vec3d eye_pos = Vec3d(0, 1.5, 2.5);

        for (int view = 0; view < view_num; view++) {


            // Draw triangle...

            // Enable depth test
            glEnable(GL_DEPTH_TEST);
            // Accept fragment if it closer to the camera than the former one
            glDepthFunc(GL_LESS);


            glfwSwapBuffers(window);

            glm::vec3 eye_pos = glm::vec3(0,0,1.3);
            glm::vec3 look_pos=glm::vec3(0,0,-1);
            glm::vec3 head_up = glm::vec3(0,1,0);

            float rotation_head_x = RandomFloat(-15,15)/180.0*3.1415926585;
            float rotation_head_y = RandomFloat(-90,90)/180.0*3.1415926585;
            float rotation_head_z = RandomFloat(-15,15)/180.0*3.1415926585;
            glm::vec3 rotation_head_vector =  glm::vec3(rotation_head_x, rotation_head_z, rotation_head_y);

            float translation_head_x = RandomFloat(-0.1,0.1);
            float translation_head_y = RandomFloat(-0.1,0.1);
            float translation_head_z = RandomFloat(-0.4,0);
            if(hair_max_strand_len > 0.8) {
                translation_head_y = RandomFloat(-0.05,0.1);
                translation_head_z = RandomFloat(-0.6, -0.1);
            }
            glm::vec3 translation_head_vector = glm::vec3(translation_head_x, translation_head_y, translation_head_z);


            glm::mat4 MVP;
            MVP = renderer.set_camera(programID, rotation_head_vector,translation_head_vector);

            glm::mat4 model_matrix = renderer.get_model_matrix(rotation_head_vector, translation_head_vector);

            renderer.save_Model_matrix(translation_head_vector, rotation_head_vector,
                                        orient_folder + hair_name + "_v" + to_string(view) + ".txt");


            cout<<"set vertex color buffer\n";
            renderer.set_vertex_color_buffer(hair, vertexbuffer, colorbuffer, MVP, Vec3d(eye_pos[0], eye_pos[1],eye_pos[2]), 1 );


            glDrawArrays(GL_LINES, 0,
                         renderer.hair_vertex_buffer_size); // Starting from vertex 0; 3 vertices total -> 1 triangle

            glDrawArrays(GL_TRIANGLES, renderer.hair_vertex_buffer_size,
                         renderer.hair_vertex_buffer_size + renderer.body_vertex_buffer_size);

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);


            glfwPollEvents();


            string orient_fn = orient_folder + hair_name + "_v" + to_string(view);

            renderer.savecurrentImg(orient_fn, 256, 256);
            //renderer.savecurrentDepthImg(orient_folder+hair_name + "_v" + to_string(view)+"_depth.png" ,256,256);
            //renderer.savecurrentDepthImg(orient_folder+hair_name + "_v" + to_string(view)+"_depth2.png" ,256,256, hair_convdata);
            hg.write_hairconv_visibility_map(hair_convdata, MVP, model_matrix,orient_folder+hair_name + "_v" + to_string(view)+".vismap" );

            //glClearBufferData(GL_ARRAY_BUFFER,GL_RGB,GL_RGB,GL_FLOAT,&vertexbuffer);
            //glClearBufferData(GL_ARRAY_BUFFER,GL_RGB,GL_RGB,GL_FLOAT,&colorbuffer);
        }

        //renderer.delete_buffer_data();


    }

    cout<<"Bad hair number: "<<sum_bad_hair<<"\n";



    glfwTerminate();
}


int main(int argc, char**argv) {


    Hairdata_generator hg;

    string hair_folder=argv[1];//"/home/yi/Documents/Hair_clean/data_generation/blend_hairs/";
    string convdata_folder=argv[2];//"/home/yi/Documents/Hair_clean/data_generation/blend_hairs_convdata/";
    string map_roots_fn=argv[3];//"/home/yi/Documents/Hair_clean/data_generation/roots1024/map_roots1024.data";

    mkdir(convdata_folder.data(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);




    generate_trainingData(hair_folder, convdata_folder, map_roots_fn);


    string orient_img_folder=argv[4];//"/home/yi/Documents/Hair_clean/data_generation/blend_hairs_imgs/";
    mkdir(orient_img_folder.data(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    int view_num=atoi( argv[5] );//4;


    visualize_and_save_hair_orient(hair_folder,convdata_folder, orient_img_folder,view_num);



    return 0;


}
