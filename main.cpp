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
    cv::Vec3b meancolor;
public:
    SuperPixel(Coo _coordonnees, cv::Vec3b  _intensite) : Pixel(_coordonnees, _intensite), meancolor(cv::Vec3b(0,0,0)){}
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
    void computeMeanColor(){
        if(pixels.size() != 0){
            int r=0,g=0,b=0;
            for(auto i : pixels){
                b += i.getIntensite().val[0];
                g += i.getIntensite().val[1];
                r += i.getIntensite().val[2];
            }
            meancolor = cv::Vec3b(b/ pixels.size(), g/ pixels.size(), r/ pixels.size());
        }
    }

    const cv::Vec3b getMeanColor() const{
        return meancolor;
    }
};


int distance(cv::Vec3b pix1, Coo coo1, cv::Vec3b pix2, Coo coo2){

    float dist_spatiale = sqrt(pow(coo2.x - coo1.x, 2) + pow(coo2.y - coo1.x,2));
    float dist_color = sqrt(pow(pix1.val[0] - pix2.val[0],2) + pow(pix1.val[1] - pix2.val[1],2) + pow(pix1.val[2] - pix2.val[2],2));
    return dist_color + dist_spatiale;
}

std::vector<SuperPixel*> sp_vec;

int main(int argc, char *argv[])
{

    cv::Mat img = cv::imread("lyon.jpg");
    int nbSuperPixel = 144; //sqrt(nbSuperPixel) doit être entier
    int dx = int(img.rows / sqrt(nbSuperPixel));
    int dy = int(img.cols / sqrt(nbSuperPixel));
    std::vector<Coo> cooSuperPixel; // a suppr
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



    for(int a = 0; a < 10; ++a)
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
                }
                sp.at<uchar>(i,j) = centroide;
                sp_vec.at(centroide)->addPixel(Pixel(Coo(i,j), img.at<cv::Vec3b>(i,j)));
            }
        }

        for(auto i : sp_vec)
            i->computeMeanColor();
        for(int i = 0; i < sp.rows; ++i)
        {
            for(int j = 0; j < sp.cols; ++j)
            {
                //egalisation des intensités des regions pr chaque superpixel
//                cv::Vec3b intensite(255*(float(sp.at<uchar>(i,j))/nbSuperPixel),0,0);
//                res.at<cv::Vec3b>(i,j) = intensite;
                res.at<cv::Vec3b>(i,j) = sp_vec.at(sp.at<uchar>(i,j))->getMeanColor();
            }
        }


        for(auto i : sp_vec){
                i->update(img);
        }
        for(int i = 1;  i < res.rows-1; ++i)
        {
            for(int j = 1; j < res.cols-1; ++j)
            {
                int currentPixel = int(sp.at<uchar>(i,j));
                if(currentPixel != int(sp.at<uchar>(i-1,j-1)) ||
                    currentPixel != int(sp.at<uchar>(i-1,j)) ||
                    currentPixel != int(sp.at<uchar>(i-1,j+1)) ||
                    currentPixel != int(sp.at<uchar>(i,j-1)) ||
                    currentPixel != int(sp.at<uchar>(i,j+1)) ||
                    currentPixel != int(sp.at<uchar>(i+1,j-1)) ||
                    currentPixel != int(sp.at<uchar>(i+1,j)) ||
                    currentPixel != int(sp.at<uchar>(i+1,j+1)))
                    res.at<cv::Vec3b>(i,j) = cv::Vec3b(0,0,0);
            }
        }
        if(a == 0 || a == 9){
        std::string numero = std::to_string(a);
        cv::namedWindow( "res_"+numero);
        cv::imshow( "res_"+numero, res);
        }
    }
    cv::waitKey(0);
    return 0;
}
