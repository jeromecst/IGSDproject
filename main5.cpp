// Inclut les en-têtes standards
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#endif

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

const int nbVertices = 842400;
const int nbVerticesVS = 336960;
const float PI = glm::pi<float>();
const float rayon = .012;
const float increment = PI / 4;
const float increment1 = PI / 8;
const float increment2 = PI / 2;
const int nbEtapesPourSigmoid = 11;
const int nombrePointsParBoucle = 18 * (nbEtapesPourSigmoid + 1);
float ecart = .04;
float largeurArc = .005;
bool disableTaux = false, D3 = false;

// stocke les variables uniformes qui seront communes a tous les vertex dessines
GLint uniform_proj, uniform_view, uniform_model;

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {

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
    } else {
        printf("Impossible to open %s. Are you in the right directory ? Don't "
               "forget to read the FAQ !\n", vertex_file_path);
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
    char const *VertexSourcePointer = VertexShaderCode.c_str();
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
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
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

void loadData(const string &fn, vector<int> &pts, vector<int> &ranks, vector<vector<int>> &vsTeam) {
    ifstream file(fn);
    int compteur = 5;
    string ptsString, ranksString, value;
    getline(file, value, ',');
    value.erase(remove(value.begin(), value.end(), ' '), value.end());
    string teamName = value;
    for (int i = 0; i < 5; i++)
        getline(file, value, ',');
    int indiceVsTeam = 0;

    while (file.good()) {
        if (compteur == 246) {
            getline(file, value, ',');
            value.erase(remove(value.begin(), value.end(), ' '), value.end());
            value.erase(remove(value.begin(), value.end(), '\n'), value.end());
            teamName = value;
            for (int i = 0; i < 5; i++)
                getline(file, value, ',');
            compteur = 5;
            indiceVsTeam += 1;
        }
        getline(file, value, ',');
        if (compteur % 6 == 0) {
            ranksString = string(value, 0, value.length());
            ranks.push_back(stoi(ranksString));
        } else if (compteur % 6 == 1) {
            ptsString = string(value, 0, value.length());
            pts.push_back(stoi(ptsString));
        } else if (compteur % 3 == 2) {
            value.erase(remove(value.begin(), value.end(), ' '), value.end());
            if (value != teamName) {
                if (value == "ManCity")
                    vsTeam[indiceVsTeam].push_back(0);
                if (value == "Liverpool")
                    vsTeam[indiceVsTeam].push_back(1);
                if (value == "Chelsea")
                    vsTeam[indiceVsTeam].push_back(2);
                if (value == "Tottenham")
                    vsTeam[indiceVsTeam].push_back(3);
                if (value == "Arsenal")
                    vsTeam[indiceVsTeam].push_back(4);
                if (value == "ManUnited")
                    vsTeam[indiceVsTeam].push_back(5);
                if (value == "Wolves")
                    vsTeam[indiceVsTeam].push_back(6);
                if (value == "Everton")
                    vsTeam[indiceVsTeam].push_back(7);
                if (value == "Leicester")
                    vsTeam[indiceVsTeam].push_back(8);
                if (value == "WestHam")
                    vsTeam[indiceVsTeam].push_back(9);
                if (value == "Watford")
                    vsTeam[indiceVsTeam].push_back(10);
                if (value == "CrystalPalace")
                    vsTeam[indiceVsTeam].push_back(11);
                if (value == "Newcastle")
                    vsTeam[indiceVsTeam].push_back(12);
                if (value == "Bournemouth")
                    vsTeam[indiceVsTeam].push_back(13);
                if (value == "Burnley")
                    vsTeam[indiceVsTeam].push_back(14);
                if (value == "Southampton")
                    vsTeam[indiceVsTeam].push_back(15);
                if (value == "Brighton")
                    vsTeam[indiceVsTeam].push_back(16);
                if (value == "Cardiff")
                    vsTeam[indiceVsTeam].push_back(17);
                if (value == "Fulham")
                    vsTeam[indiceVsTeam].push_back(18);
                if (value == "Huddersfield")
                    vsTeam[indiceVsTeam].push_back(19);
                if (value == "??")
                    vsTeam[indiceVsTeam].push_back((-1));
            }
        }
        compteur += 1;
    }
}

vector<vector<float>> initializeY(vector<int> &ranks, vector<int> &pts) {
    vector<vector<float>> y = vector<vector<float >>(20);
    int o = 0;
    for (int team = 0; team < 20; team++) {
        y[team] = vector<float>(0);
        for (int jour = 0; jour < 40; jour++) {
            y[team].push_back(((19 - ranks[jour + o]) / 19. + (pts[jour + o] / 98.)) / 2.);
        }
        o += 40;
    }
    return y;
}

float sigmoid(float x, float a, float b) {
    return a * (1. / (1. + glm::exp(-x))) + b;
}

vector<float> initializeX(int size) {
    vector<float> x = vector<float>(size);
    for (int i = 0; i < size; i++) {
        x[i] = (float) i / (float) size;
    }
    return x;
}

vector<int> finalRanking(vector<int> ranks) {
    vector<int> finalRanks = vector<int>(20);
    for (int i = 0; i < 20; i++) {
        finalRanks[ranks[40 * i + 39]] = i;
    }
    return finalRanks;
}

void fillVertex(GLfloat g_vertex_buffer_data[], GLfloat g_vertex_color_data[], const vector<vector<float>> &y, const vector<float> &x, int teamSelected) {
    float tabX[] = {-5.5, -3.6, -2.3, -1.6, -1., 0., 1., 1.6, 2.3, 3.6, 5.5};

    float selectedZ = 0, colorAdd, taux;
    int k = 0;
    for (int team = 0; team < (int) y.size(); team++) {
        for (int jour = 0; jour < (int) x.size() - 1; jour++) {
            /// drawing 1 quad

            for (float theta = PI; theta > -increment; theta -= increment) {
                /// 3 points du premier triangle
                //1
                g_vertex_buffer_data[k] = (x[jour] + jour * ecart);
                g_vertex_buffer_data[k + 1] = (y[team][jour] + rayon * cos(theta));
                g_vertex_buffer_data[k + 2] = (rayon * sin(theta) + selectedZ);

                //2
                g_vertex_buffer_data[k + 3] = (x[jour + 1] + jour * ecart);
                g_vertex_buffer_data[k + 4] = (y[team][jour] + rayon * cos(theta));
                g_vertex_buffer_data[k + 5] = (rayon * sin(theta) + selectedZ);

                //3
                g_vertex_buffer_data[k + 6] = (x[jour] + jour * ecart);
                g_vertex_buffer_data[k + 7] = (y[team][jour] + rayon * cos(theta + increment));
                g_vertex_buffer_data[k + 8] = (rayon * sin(theta + increment) + selectedZ);

                //4
                g_vertex_buffer_data[k + 9] = (x[jour + 1] + jour * ecart);
                g_vertex_buffer_data[k + 10] = (y[team][jour] + rayon * cos(theta));
                g_vertex_buffer_data[k + 11] = (rayon * sin(theta) + selectedZ);

                //5
                g_vertex_buffer_data[k + 12] = (x[jour] + jour * ecart);
                g_vertex_buffer_data[k + 13] = (y[team][jour] + rayon * cos(theta + increment));
                g_vertex_buffer_data[k + 14] = (rayon * sin(theta + increment) + selectedZ);

                //6
                g_vertex_buffer_data[k + 15] = (x[jour + 1] + jour * ecart);
                g_vertex_buffer_data[k + 16] = (y[team][jour] + rayon * cos(theta + increment));
                g_vertex_buffer_data[k + 17] = (rayon * sin(theta + increment) + selectedZ);

                if ((y[team][jour] < y[team][jour + 1] && !D3) || (D3 && !disableTaux)) {
                    taux = (y[team][jour + 1] - y[team][jour]) + .01;
                } else if (team == teamSelected or !D3) {
                    taux = (y[team][jour + 1] - y[team][jour]) + .01;
                } else {
                    taux = 0;
                }

                k += 18;
                float tabTaux[nbEtapesPourSigmoid];
                float inc = PI / nbEtapesPourSigmoid;
                for (int i = 0; i < nbEtapesPourSigmoid; i++) {
                    tabTaux[i] = taux * sin((float) i * inc);
                }
                float yavant = y[team][jour];
                float yapres = y[team][jour + 1];
                float tauxVariation = yapres - yavant;

                for (int nbPente = 0; nbPente < nbEtapesPourSigmoid; nbPente++) {
                    float sigI = sigmoid(tabX[nbPente], tauxVariation, yavant);
                    float sigII = sigmoid(tabX[nbPente + 1], tauxVariation, yavant);

                    //7
                    g_vertex_buffer_data[k] = (x[jour + 1] + (float) jour * ecart + ((float) nbPente * ecart / nbEtapesPourSigmoid));
                    if (nbPente == 0) {
                        g_vertex_buffer_data[k + 1] = (y[team][jour] + rayon * cos(theta));
                    } else {
                        g_vertex_buffer_data[k + 1] = (sigI) + rayon * cos(theta);
                    }
                    g_vertex_buffer_data[k + 2] = (rayon * sin(theta) + selectedZ + tabTaux[nbPente]);

                    //8
                    g_vertex_buffer_data[k + 3] = (x[jour + 1] + (float) jour * ecart + (((float) nbPente + 1.) * ecart / nbEtapesPourSigmoid));
                    if (nbPente == nbEtapesPourSigmoid - 1) {
                        g_vertex_buffer_data[k + 4] = (y[team][jour + 1] + rayon * cos(theta));
                    } else {
                        g_vertex_buffer_data[k + 4] = (sigII) + rayon * cos(theta);
                    }
                    g_vertex_buffer_data[k + 5] = (rayon * sin(theta) + selectedZ + tabTaux[nbPente + 1]);


                    //9
                    g_vertex_buffer_data[k + 6] = (x[jour + 1] + (float) jour * ecart + ((float) nbPente * ecart / nbEtapesPourSigmoid));
                    if (nbPente == 0) {
                        g_vertex_buffer_data[k + 7] = (y[team][jour] + rayon * cos(theta + increment));
                    } else {
                        g_vertex_buffer_data[k + 7] = (sigI) + rayon * cos(theta + increment);
                    }
                    g_vertex_buffer_data[k + 8] = (rayon * sin(theta + increment) + selectedZ + tabTaux[nbPente]);

                    //10
                    g_vertex_buffer_data[k + 9] = (x[jour + 1] + (float) jour * ecart + (((float) nbPente + 1.) * ecart / nbEtapesPourSigmoid));
                    if (nbPente == nbEtapesPourSigmoid - 1) {
                        g_vertex_buffer_data[k + 10] = (y[team][jour + 1] + rayon * cos(theta));
                    } else {
                        g_vertex_buffer_data[k + 10] = (sigII) + rayon * cos(theta);
                    }
                    g_vertex_buffer_data[k + 11] = (rayon * sin(theta) + selectedZ + tabTaux[nbPente + 1]);

                    //11
                    g_vertex_buffer_data[k + 12] = (x[jour + 1] + (float) jour * ecart + ((float) nbPente * ecart / nbEtapesPourSigmoid));
                    if (nbPente == 0) {
                        g_vertex_buffer_data[k + 13] = (y[team][jour] + rayon * cos(theta + increment));
                    } else {
                        g_vertex_buffer_data[k + 13] = (sigI) + rayon * cos(theta + increment);
                    }
                    g_vertex_buffer_data[k + 14] = (rayon * sin(theta + increment) + selectedZ + tabTaux[nbPente]);

                    //12
                    g_vertex_buffer_data[k + 15] = (x[jour + 1] + (float) jour * ecart + ((float) (nbPente + 1.) * ecart / nbEtapesPourSigmoid));
                    if (nbPente == nbEtapesPourSigmoid - 1) {
                        g_vertex_buffer_data[k + 16] = (y[team][jour + 1] + rayon * cos(theta + increment));
                    } else {
                        g_vertex_buffer_data[k + 16] = (sigII) + rayon * cos(theta + increment);
                    }
                    g_vertex_buffer_data[k + 17] = (rayon * sin(theta + increment) + selectedZ + tabTaux[nbPente + 1]);

                    k += 18;
                }

                k -= nombrePointsParBoucle;
                int v = 0;
                if (team == teamSelected && (theta > PI - 6. * increment || (theta < 6. * increment))) {
                    colorAdd = 0;
                } else if (teamSelected == team) {
                    colorAdd = .6;
                } else {
                    colorAdd = 1;
                }

                for (int i = 0; i < nombrePointsParBoucle / 3; i++) {
                    if (i % 6 == 0 or i % 6 == 1 or i % 6 == 3) {
                        if (team >= 0 and team < 4) {
                            // BLEU
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - theta / PI) * colorAdd;
                            v += 1;
                        } else if (team >= 4 and team < 7) {
                            // JAUNE
                            g_vertex_color_data[k + v] = (1 - theta / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - theta / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                        } else if (team >= 7 and team < 16) { // CYAN
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - theta / PI) * .8 * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - theta / PI) * colorAdd;
                            v += 1;

                        } else {
                            // ROUGE
                            g_vertex_color_data[k + v] = (1 - theta / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                        }

                    } else {
                        if (team >= 0 and team < 4) {
                            // BLEU
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * colorAdd;
                            v += 1;
                        } else if (team >= 4 and team < 7) {
                            // JAUNE
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                        } else if (team >= 7 and team < 16) { // CYAN
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * .8 * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * colorAdd;
                            v += 1;

                        } else {
                            // ROUGE
                            g_vertex_color_data[k + v] = (1 - (theta + increment) / PI) * colorAdd;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                            g_vertex_color_data[k + v] = 0;
                            v += 1;
                        }
                    }

                }
                k += nombrePointsParBoucle;

            }
        }
    }
}

void fillVsTeamVertex(GLfloat g_vertex_buffer_dataVS[], vector<float> x, const vector<vector<float>> &y, const vector<vector<int>> &vsTeam, int teamSelected) {
    float milieu = 0, rayyon = 0;
    int k = 0;
    for (int jour = 0; jour < x.size() - 1; jour++) {
        if (vsTeam[teamSelected][jour] != -1) {
            milieu = (y[teamSelected][jour] + y[vsTeam[teamSelected][jour]][jour]) / 2;
            rayyon = abs(y[teamSelected][jour] - y[vsTeam[teamSelected][jour]][jour]) / 2;
        } else {
            milieu = 0;
            rayyon = 0;

        }
        for (float theta = PI - increment1; theta > 0; theta -= increment1) {
            for (float theta2 = 0; theta2 < 2 * PI - increment2; theta2 += increment1) {
                g_vertex_buffer_dataVS[k + 0] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2) * largeurArc;
                g_vertex_buffer_dataVS[k + 1] = milieu + rayyon * cos(theta);
                g_vertex_buffer_dataVS[k + 2] = rayyon * sin(theta) + largeurArc * sin(theta2);

                g_vertex_buffer_dataVS[k + 3] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2) * largeurArc;
                g_vertex_buffer_dataVS[k + 4] = milieu + rayyon * cos(theta + increment1);
                g_vertex_buffer_dataVS[k + 5] = rayyon * sin(theta + increment1) + largeurArc * sin(theta2);

                g_vertex_buffer_dataVS[k + 6] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2 + increment2) * largeurArc;
                g_vertex_buffer_dataVS[k + 7] = milieu + rayyon * cos(theta + increment1);
                g_vertex_buffer_dataVS[k + 8] = rayyon * sin(theta + increment1) + largeurArc * sin(theta2 + increment2);

                ///

                g_vertex_buffer_dataVS[k + 9] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2) * largeurArc;
                g_vertex_buffer_dataVS[k + 10] = milieu + rayyon * cos(theta);
                g_vertex_buffer_dataVS[k + 11] = rayyon * sin(theta) + largeurArc * sin(theta2);

                g_vertex_buffer_dataVS[k + 12] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2 + increment2) * largeurArc;
                g_vertex_buffer_dataVS[k + 13] = milieu + rayyon * cos(theta);
                g_vertex_buffer_dataVS[k + 14] = rayyon * sin(theta) + largeurArc * sin(theta2 + increment2);

                g_vertex_buffer_dataVS[k + 15] = ((x[jour] + x[jour + 1]) / 2 + jour * ecart) + cos(theta2 + increment2) * largeurArc;
                g_vertex_buffer_dataVS[k + 16] = milieu + rayyon * cos(theta + increment1);
                g_vertex_buffer_dataVS[k + 17] = rayyon * sin(theta + increment1) + largeurArc * sin(theta2 + increment2);

                k += 18;
            }
        }
    }
}

int sizeVertex(int xSize, int ySize) {
    int k = 0;
    for (float theta = PI; theta > -increment; theta -= increment) {
        k += 1;
    }
    return ySize * (xSize - 1) * nombrePointsParBoucle * k;
}

int sizeVertexVS(int xSize, int ySize) {
    int k = 0, j = 0;
    for (float theta = PI - increment1; theta > 0; theta -= increment1) {
        k += 1;
    }
    for (float theta = 0; theta < 2 * PI - increment2; theta += increment2) {
        j += 1;
    }
    return ySize * (xSize - 1) * 18 * k * j;
}

void emptyVsTeamVertex(GLfloat g_vertex_buffer_dataVS[], int xSize, int ySize) {
    int size = sizeVertexVS(xSize, ySize);
    for (int i = 0; i < size; i++) {
        g_vertex_buffer_dataVS[i] = 0;
    }
}

float angled = 0.0f;
float xLookCam = 1.3;
float yLookCam = .5;
float theta = .3;
float omega = 1.5;
bool updateVertex = true;
bool rotated = false;
bool first = true;
double xpos1 = 0, xpos2, ypos1 = 0, ypos2;
int teamSelected = -1, tic = 0;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (yoffset == 1) {
        omega -= .06f * omega;
    } else {
        omega += .06f * omega;
    }
}

void setKeyCallback(GLFWwindow *window) {

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS or glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (D3) {
            angled += .01;

        } else {
            xLookCam += .01;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS or glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (D3) {
            angled -= .01;

        } else {
            xLookCam -= .01;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS or glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (D3) {
            theta -= .02;

        } else {
            yLookCam += .01;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS or glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (D3) {
            theta += .02;

        } else {
            yLookCam -= .01;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        ecart += .001;
        updateVertex = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        ecart -= .001;
        updateVertex = true;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS or
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_5) == GLFW_PRESS) {
        omega -= .01;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        omega += .01;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        teamSelected = -1;
        disableTaux = false;
        updateVertex = true;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && tic > 8) {
        disableTaux = true;
        tic = 0;
        teamSelected -= 1;
        if (teamSelected < 0) {
            teamSelected = 19;
        }
        teamSelected = teamSelected % 20;
        updateVertex = true;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && tic > 8) {
        disableTaux = true;
        tic = 0;
        teamSelected += 1;
        teamSelected = teamSelected % 20;
        updateVertex = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && tic > 10) {
        tic = 0;
        if (not D3) {
            D3 = true;
            theta = -1;
        } else {
            angled = 0.0f;
            theta = .3;
            omega = 2;
            rotated = false;
            D3 = false;
        }
        updateVertex = true;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && tic > 14) {
        rotated = not rotated;
        tic = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        largeurArc += .001;
        updateVertex = true;
    }

    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        largeurArc -= .001;
        updateVertex = true;
    }

    if (omega < 0) {
        omega = 0;
    }
    if (theta > 0) {
        theta = 0;
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (first) {
            glfwGetCursorPos(window, &xpos1, &ypos1);
        }
        xpos2 = xpos1;
        ypos2 = ypos1;
        first = false;
        glfwGetCursorPos(window, &xpos1, &ypos1);
        if (D3) {
            if (xpos1 > xpos2) {
                angled += PI / 200 * abs(xpos1 - xpos2);
            } else if (xpos1 < xpos2) {
                angled -= PI / 200 * abs(xpos1 - xpos2);
            }

        } else {
            xLookCam += (xpos2 - xpos1) * omega / 600;
            yLookCam += (ypos1 - ypos2) * omega / 600;
        }
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && D3) {
        if (first) {
            glfwGetCursorPos(window, &xpos1, &ypos1);
        }
        xpos2 = xpos1;
        ypos2 = ypos1;
        first = false;
        glfwGetCursorPos(window, &xpos1, &ypos1);
        xLookCam += (xpos2 - xpos1) * omega / 500;
    } else {
        first = true;
    }
}

int main() {
    vector<int> ranks = vector<int>(0);
    vector<int> pts = vector<int>(0);
    vector<vector<int>> vsTeam = vector<vector<int >>(20);
    loadData("rankspts.csv", pts, ranks, vsTeam);
    vector<vector<float>> y = initializeY(ranks, pts);
    vector<float> x = initializeX(y[0].size());
    vector<int> finalRanks = finalRanking(ranks);
    cout << sizeVertex(x.size(), y.size()) << endl;
    cout << sizeVertexVS(x.size(), y.size()) << endl;
    GLfloat g_vertex_buffer_data[nbVertices];
    GLfloat g_vertex_color_data[nbVertices];
    GLfloat g_vertex_buffer_dataVS[nbVerticesVS];
    fillVertex(g_vertex_buffer_data, g_vertex_color_data, y, x, -1);
    fillVsTeamVertex(g_vertex_buffer_dataVS, x, y, vsTeam, 1);

    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // On veut OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                   GL_TRUE); // Pour rendre MacOS heureux ; ne devrait pas être nécessaire
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // On ne veut pas l'ancien OpenGL
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // Ouvre une fenêtre et crée son contexte OpenGl
    GLFWwindow *window; // (Dans le code source qui accompagne, cette variable est
    // globale)
    window = glfwCreateWindow(1920, 1080, "ProjetIG", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are "
                        "not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); // Initialise GLEW
    glewExperimental = true;        // Nécessaire dans le profil de base
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Bon maintenant on cree le VAO et cette fois on va s'en servir !
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);

    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Only allocqte memory. Do not send yet our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(g_vertex_buffer_data) + sizeof(g_vertex_color_data) + sizeof(g_vertex_buffer_dataVS),
                 nullptr, GL_STATIC_DRAW);

    // send vertices in the first part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);

    // send colors in the second part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), sizeof(g_vertex_color_data),
                    g_vertex_color_data);

    // send colors in the third part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_color_data) + sizeof(g_vertex_buffer_data),
                    sizeof(g_vertex_buffer_dataVS), g_vertex_buffer_dataVS);

    // ici les commandes stockees "une fois pour toute" dans le VAO
    glVertexAttribPointer(0, // attribute 0. No particular reason for 0, but must
            // match the layout in the shader.
                          3, // size
                          GL_FLOAT,       // type
                          GL_FALSE,       // normalized?
                          0,              // stride
                          (void *) nullptr // array buffer offset
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer( // same thing for the colors
            1, 3, GL_FLOAT, GL_FALSE, 0, (void *) sizeof(g_vertex_buffer_data));

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, // attribute 0. No particular reason for 0, but must match the layout
            // in the shader.
                          3, // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0,        // stride
                          (void *) (sizeof(g_vertex_buffer_data) +
                                  sizeof(g_vertex_color_data)) // array buffer offset
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // on desactive le VAO a la fin de l'initialisation
    glBindVertexArray(0);

    // Assure que l'on peut capturer la touche d'échappement enfoncée ci-dessous
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    GLuint ProgramID = LoadShaders("SimpleVertexShader5.vertexshader", "SimpleFragmentShader5.fragmentshader");
    uniform_proj = glGetUniformLocation(ProgramID, "projectionMatrix");
    uniform_view = glGetUniformLocation(ProgramID, "viewMatrix");
    uniform_model = glGetUniformLocation(ProgramID, "modelMatrix");

    glm::mat4 projectionMatrix, viewMatrix;
    mat4 modelMatrix;

    do {
        tic += 1;

        // Use our shader program
        glUseProgram(ProgramID);

        // SET BACKGROUND COLOR TO WHITE
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1, 1, 1, 0);

        projectionMatrix = glm::ortho(-omega, omega, omega, -omega, -20.f, 9.f);
        if (D3) {
            if (rotated) {
                angled += 0.004f;
            }
            viewMatrix = glm::lookAt(vec3(cos(angled) + xLookCam, sin(angled) + yLookCam,
                                          theta),                 // where is the camera
                                     vec3(xLookCam, yLookCam, 0), // where it looks
                                     vec3(0, 0, -1)               // head is up
            );
        } else {
            //2D
            viewMatrix = glm::lookAt(vec3(xLookCam, yLookCam + 10e-5, -1), // where is the camera
                                     vec3(xLookCam, yLookCam, 0),           // where it looks
                                     vec3(0, 0, -1)                         // head is up
            );
        }
        modelMatrix = glm::mat4(1.0);
        glUniformMatrix4fv(uniform_proj, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        // on re-active le VAO avant d'envoyer les buffers
        glBindVertexArray(VertexArrayID);

        // Draw the triangle(s) !
        glDrawArrays(GL_TRIANGLES, 0,
                     (sizeof(g_vertex_buffer_dataVS) + sizeof(g_vertex_buffer_data) + sizeof(g_vertex_color_data)) /
                             (3 * sizeof(float))); // Starting from vertex 0 .. all the buffer

        // on desactive le VAO a la fin du dessin
        glBindVertexArray(0);

        // on desactive les shaders
        glUseProgram(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Inputs
        glfwSetScrollCallback(window, scroll_callback);
        setKeyCallback(window);
        glfwSetCursorPosCallback(window, cursor_position_callback);

        if (updateVertex) {
            fillVertex(g_vertex_buffer_data, g_vertex_color_data, y, x, teamSelected);
            if (teamSelected > -1)
                fillVsTeamVertex(g_vertex_buffer_dataVS, x, y, vsTeam, teamSelected);
            else {
                emptyVsTeamVertex(g_vertex_buffer_dataVS, x.size(), y.size());
            }
        }
        updateVertex = false;

        // The following commands will talk about our 'vertexbuffer' buffer
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

        // Only allocqte memory. Do not send yet our vertices to OpenGL.
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data) + sizeof(g_vertex_color_data) +
                sizeof(g_vertex_buffer_dataVS), nullptr, GL_STATIC_DRAW);

        // send vertices in the first part of the buffer
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);

        // send colors in the second part of the buffer
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), sizeof(g_vertex_color_data),
                        g_vertex_color_data);

        // send arc de cercles in the third part of the buffer
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(g_vertex_color_data) + sizeof(g_vertex_buffer_data),
                        sizeof(g_vertex_buffer_dataVS), g_vertex_buffer_dataVS);

    } // Vérifie si on a appuyé sur la touche échap (ESC) ou si la fenêtre a été
        // fermée
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}
