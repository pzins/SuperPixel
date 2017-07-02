#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>


using namespace std;

struct Coo {
    int x;
    int y;
    Coo(int _x, int _y){x = _x; y = _y;}
};
void printCoo(Coo c){std::cout << c.x << " " << c.y << std::endl;}

class Pixel
{
protected:
    Coo coordonnees;
    cv::Vec3b intensite;
public:
    Pixel(Coo _coordonnees, cv::Vec3b _intensite) : coordonnees(_coordonnees), intensite(_intensite){}
    const Coo getCoordonnees(){return coordonnees;}
    const cv::Vec3b getIntensite(){return intensite;}
};

class SuperPixel : public Pixel
{
private:
    std::vector<Pixel> pixels;
    float meanR;
    float meanG;
    float meanB;
public:
    SuperPixel(Coo _coordonnees, cv::Vec3b  _intensite) : Pixel(_coordonnees, _intensite), meanR(0), meanG(0), meanB(0){}
    int operator-(Pixel _pixel) {
        float dist_spatiale = sqrt(pow(_pixel.getCoordonnees().x - coordonnees.x, 2) + pow(_pixel.getCoordonnees().y - coordonnees.y,2));
        float dist_color = sqrt(pow(_pixel.getIntensite().val[0] - intensite.val[0],2) + pow(_pixel.getIntensite().val[1] - intensite.val[1],2)
                + pow(_pixel.getIntensite().val[2] - intensite.val[2],2));
        return dist_color + dist_spatiale;
    }
    void addPixel(Pixel _pix){pixels.push_back(_pix);}

    void update(cv::Mat& _img){
        float newx = 0, newy = 0;
        for(auto i : pixels)
        {
            newx += i.getCoordonnees().x;
            newy += i.getCoordonnees().y;
        }
        if(newx != 0 && newy != 0)
        {
            newx /= pixels.size();
            newy /= pixels.size();
        }
        coordonnees.x = newx;
        coordonnees.y = newy;
        pixels.clear();

        intensite = _img.at<cv::Vec3b>(coordonnees.x, coordonnees.y);
    }
    void updateMeanColor(cv::Vec3b _color){
        meanB = (meanB * pixels.size() + _color[0])/(pixels.size()+1);
        meanG = (meanG * pixels.size() + _color[1])/(pixels.size()+1);
        meanR = (meanR * pixels.size() + _color[2])/(pixels.size()+1);
    }

    const cv::Vec3b getMeanColor() const{
        return cv::Vec3b(meanB, meanG, meanR);
    }
    void printPixels(){
        for(auto i : pixels)
            std::cout << (int) i.getIntensite().val[0] << std::endl;
    }
    int getNBElements(){return pixels.size();}
};

std::vector<SuperPixel*> sp_vec;

int main(int argc, char *argv[])
{

    cv::Mat img = cv::imread("montagne1.jpg");
    int nbSuperPixel = 100; //sqrt(nbSuperPixel) doit être entier
    int dx = int(img.rows / sqrt(nbSuperPixel));
    int dy = int(img.cols / sqrt(nbSuperPixel));

    //computer superpixel points (coo + intensite)
    for(int i = 0; i < sqrt(nbSuperPixel); ++i)
    {
        for(int j = 0; j < sqrt(nbSuperPixel); ++j){
            SuperPixel* tmp = new SuperPixel(Coo((i+1)*dx,(j+1)*dy), img.at<cv::Vec3b>((i+1)*dx, (j+1)*dy));
            sp_vec.push_back(tmp);
        }
    }
    cv::Mat res(img.rows, img.cols, CV_8UC3); //matrice resultat pr l'affichage
    cv::Mat sp(img.rows, img.cols, CV_8UC1); //matrice image avec value = indice du superpixel associé



    for(int a = 0; a < 1; ++a)
    {
        //loop over image
        //compute sp : with superpixel index for each pixel
        //and add this pixel to its associated superpixel
        for(int i = 0; i < img.rows; ++i)
        {
            for(int j = 0; j < img.cols; ++j)
            {
                int centroide = -1;
                int mini_dist = 10000000;
                //loop over superpixels
                for(int k = 0; k < sp_vec.size(); ++k)
                {
                    Pixel p(Coo(i,j), img.at<cv::Vec3b>(i,j));
                    int dist = *sp_vec.at(k) - p;
                    if(dist < mini_dist)
                    {
                        centroide = k;
                        mini_dist = dist;
                    }
                    //acceleration for superpixel
                    if(dist == 0)
                        break;
                }
                sp.at<uchar>(i,j) = centroide;
                sp_vec.at(centroide)->updateMeanColor(img.at<cv::Vec3b>(i,j));
                sp_vec.at(centroide)->addPixel(Pixel(Coo(i,j), img.at<cv::Vec3b>(i,j)));
                res.at<cv::Vec3b>(i,j) = sp_vec.at(sp.at<uchar>(i,j))->getMeanColor();
            }
        }


        //peut etre en faisant ça dynamiquement à chaque ajout
        //faire une nvelle branche avec ça et
        //voir ensuite version plus rapide
        //update centroides
        for(auto i : sp_vec){
                i->update(img);
        }
        //draw outlines
        for(int i = 1;  i < res.rows-1; ++i)
        {
            for(int j = 1; j < res.cols-1; ++j)
            {
                int currentPixel = int(sp.at<uchar>(i,j));
                if(
                    currentPixel != int(sp.at<uchar>(i,j+1)) ||
                    currentPixel != int(sp.at<uchar>(i+1,j)) ||
                    currentPixel != int(sp.at<uchar>(i+1,j+1)))
                    res.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
            }
        }

        //affichage des superpixels
//        for(auto i : sp_vec)
//        {
//            Coo c = i->getCoordonnees();
//            res.at<cv::Vec3b>(c.x, c.y) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x-1, c.y-1) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x-1, c.y) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x-1, c.y+1) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x, c.y-1) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x, c.y+1) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x+1, c.y-1) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x+1, c.y) = cv::Vec3b(0,0,255);
//            res.at<cv::Vec3b>(c.x+1, c.y+1) = cv::Vec3b(0,0,255);
//        }


        //windows
        if(a == 0 || a == 9){
        std::string numero = std::to_string(a);
        cv::namedWindow( "res_"+numero);
        cv::imshow( "res_"+numero, res);
        }
    }
    cv::waitKey(0);
    return 0;
}
