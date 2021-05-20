
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types.hpp>
#include <iostream>
using namespace std;
using namespace cv;


/*************************affichage*******************************/
void afficher_image(Mat image, string nom) {
	imshow(nom, image);
}
/************************binarisation******************************/
/*Fonction qui permet de faire un binarisation d'image grace a opencv*/
Mat binarisationCV(Mat& img) {
	Mat image_f;
	adaptiveThreshold(img, image_f, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 5, 2); 
	return image_f;
}
/***********************filtre de lissage***********************/
/*filtre gaussien de taille 5*5*/
Mat GaussianFilter(Mat& image) {
	// flouter l'image avec le filtre gaussien ( kernel size = 3 )
	GaussianBlur(image, image, Size(5, 5), 0, 0, BORDER_DEFAULT); 
	
	return image;
}
/*filtre moyenneur */
Mat MoyenneurFilter(Mat& image) {
	Mat image_f;
	// flouter l'image avec le filtre moyenneur ( kernel size = 3 )
	blur(image, image_f, Size(3, 3));
	return image_f;
}

/*filtre floutage avec addition*/
Mat flouterAvecAddition(Mat& image) {
	// flouter avce la somme de l'image pricipale avec un poid de 1.5 et l'image gaussienne avec -0.5 et un gama de 0.0
	Mat image_f, gaussien;
	gaussien = GaussianFilter(image);
	double image1Weight = 1.5, image2Weight = -0.5, gama=0.0;
	addWeighted(image , image1Weight , gaussien, image2Weight, gama, image_f);
	return image_f;
}

/*fonction qui permet de detecter les lignes avec la transformer de hough*/
vector<Vec4i> ligneHough(Mat& image) {
	double rho = 1.0, theta = CV_PI/180 , min_line_length = 120, max_line_gap=40;
	int threshold = 10;
	vector<Vec4i> lines;
	HoughLinesP(image, lines, rho, theta, threshold, min_line_length, max_line_gap);
	return lines;
}


// affichage des lignes sur l'image 
void afficherImageAvecLigne(Mat image, vector<Vec4i> lines,string nom) {
	Mat color_dst;
	cvtColor(image, color_dst, COLOR_GRAY2BGR);
	for (size_t i = 0; i < lines.size(); i++)
	{
		line(color_dst, Point(lines[i][0], lines[i][1]),
		Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 3, 8);
	}
	
	afficher_image(color_dst,nom);
}

//garder que les lignes horizontaux 
vector<Vec4i> horizontalLines(vector<Vec4i>& lignes, int degree_max=0) {
	vector<Vec4i> lignesH;
	Vec4i vec;
	double degree;
	int a, b;
	for (size_t i = 0; i < lignes.size(); i++)
	{
		
		a = abs(lignes[i][1] - lignes[i][3]);
		b = abs(lignes[i][0] - lignes[i][2]);
		degree = 0;
		if (a == 0) {
			degree = 0;
		}
		if (b==0) {
			degree = 180;
		}
		else {
			degree = atan(a / b) * 180 / CV_PI;
		}
		if (degree<=degree_max) {
			  lignesH.push_back(Vec4i(lignes[i][0], lignes[i][1], lignes[i][2], lignes[i][3]));
		}
	
		
	}
	return lignesH;
}
//x1 minimum dans les lignes 
int min_x(vector<Vec4i>& lignes) {
	int minimum = lignes[0][0];
	for (size_t i = 0; i < lignes.size(); i++)
	{
		if (minimum > lignes[i][0]) {
			minimum = lignes[i][0];
		}
	}
	return minimum;
}
//y1 minimum dans les lignes 
int min_y(vector<Vec4i>& lignes) {
	int minimum = lignes[0][1];
	for (size_t i = 0; i < lignes.size(); i++)
	{
		if (minimum > lignes[i][1]) {
			minimum = lignes[i][1];
		}
	}
	return minimum;
}
 
// x2 maximum dans les lignes 
int max_x(vector<Vec4i>& lignes) {
	int maximum = lignes[0][2];
	for (size_t i = 0; i < lignes.size(); i++)
	{
		if (maximum < lignes[i][2]) {
			maximum = lignes[i][2];
		}
	}
	return maximum;
}
// y2 maximum dans les lignes 
int max_y(vector<Vec4i>& lignes) {
	int maximum = lignes[0][3];
	for (size_t i = 0; i < lignes.size(); i++)
	{
		if (maximum < lignes[i][3]) {
			maximum = lignes[i][3];
		}
	}
	return maximum;
}
bool checkVelueVI(vector<int> vecteur, int value) {
	for (vector<int>::iterator i = vecteur.begin(); i != vecteur.end();i++) {
		if (value == *i) {
			return true;
		}
	}
	return false;
}


//regrouper ligne et retourner une liste de lignes horizontaux qui regroupe tout les lignes 
vector<Vec4i> regrouperLignes(vector<Vec4i> lignes,  int seuil) {

	vector<Vec4i> lignes1, lignesF;
	vector<vector<int>> new_ligne_dic;
	vector<int> visiter;
	double dist;
	int l = 0,minx,maxx,miny,maxy,moyy;
	//reegroupement de tout les lignes avec une distance de seuille entre elles 
	for (size_t i = 0; i < lignes.size() - 1; i++)
	{
		if (checkVelueVI(visiter, i)) {
			continue;
		}
		else {
			
			seuil *= 0.9;
			new_ligne_dic.push_back(vector<int>());
			visiter.push_back(i);
			
			for (size_t j = i + 1; j < lignes.size(); j++) {
				dist = abs(lignes[i][1] - lignes[j][1]);
				if (dist< seuil) {
					new_ligne_dic.at(l).push_back(j);
					visiter.push_back(j);
				}
			}
			l++;
		}
	}

	//regroupement de toutes les lignes avec une distance seuille, nous les transformant a une seul lignes en prenant le min et le max de x, ensuite la moyenne de maxy miny pour etre au milieu 
	for (size_t i = 0; i < new_ligne_dic.size(); i++)
	{
		for (size_t j = 0; j < new_ligne_dic.at(i).size(); j++)
		{
			lignes1.push_back(lignes.at(new_ligne_dic.at(i).at(j)));
		}
		minx = min_x(lignes1);
		maxx = max_x(lignes1);
		miny = min_y(lignes1);
		maxy = max_y(lignes1);
		moyy = int( miny+(maxy-miny)/2);
		lignesF.push_back(Vec4i(minx, moyy,maxx,moyy));
		lignes1.clear();
	}
	return lignesF;
}


//detection de la ligne la qui contient le y minimum et supperieur a 60 (20 pourcent )
Vec4i niveauEau(vector<Vec4i>& lignes) {
	Vec4i niveauE;
	//prendre que les lignes a partir de 20 pourcent de notre verre
	int pourcentageVideHaut = 60;
	int minimum = 450;
	
	for (size_t i = 0; i < lignes.size(); i++)
	{
		
		if (minimum > lignes[i][1] && minimum >= pourcentageVideHaut && lignes[i][1]>= pourcentageVideHaut) {
			minimum = lignes[i][1];
			niveauE = lignes.at(i);
		}
	}
	return niveauE;
}
// detection de la ligne qui contiant le y maximum
Vec4i BasVerre(vector<Vec4i>& lignes) {
	Vec4i niveauV;
	int maximum = 0;
	for (size_t i = 0; i < lignes.size(); i++)
	{
		if (maximum < lignes[i][1]) {
			maximum = lignes[i][1];
			niveauV = lignes.at(i);
		}
	}
	return niveauV;
}
// on pose que 15 pourcent du verre d'en haut est vide et on selectionne la premiere ligne apres les 15 pourcent qui est le niveau d'eau et la derniere ligne qui est le bas du verre 
vector<Vec4i> trierLignes(vector<Vec4i> lignes) {
	Vec4i niveau_Eau = niveauEau(lignes); //detection du niveau d'eau 
	Vec4i niveau_Bas_Verre = BasVerre(lignes);//detection du bas du verre
	vector<Vec4i> resultFinal;
	resultFinal.push_back(niveau_Eau);
	resultFinal.push_back(niveau_Bas_Verre);
	return resultFinal;
}
// remplir l'interface de l'eau avec une couleur bleur
Mat remplirSurface(Mat& image, vector<Vec4i> vecteur) {
	
	Mat imageF;
	cvtColor(image, imageF, COLOR_GRAY2BGR);
	Rect rec = Rect(vecteur[0][0], vecteur[0][1], vecteur[0][2], vecteur[1][3]);
	cout << vecteur.at(0)<< endl<<vecteur.at(1);
	//dessiner le rectangle
	rectangle(imageF,rec, Scalar(255, 0, 0), 3,8 ,0);
	int index = vecteur[0][1];
	while (index <= vecteur[1][3])
	{
		//remplir le rectangle
		line(imageF, Point(vecteur[0][0], index), Point(vecteur[0][2], index), Scalar(255, 0, 0), 2, 8, 0);
		index+=10;
	}
	return imageF;
}
int main()
{
	/****************variables**************/
	Mat image1, LoadedImage, gray, gaussien, moyenneur, bin, somme , avecRec;
	vector<Vec4i> lines,lignesH, ligneHRegroupe, ligneFinal;

	/*******************Etape1*********************/
	//charger l'image et la mettre en gris directement 
	image1 = cv::imread("C:/Users/younes/Desktop/projetImage/7.jpg", IMREAD_COLOR);
	//afficher_image(image1, "originale");
	//redimensionner l'image en 300*450
	resize(image1, LoadedImage, Size(300, 450));
	afficher_image(LoadedImage, "originale");
	//transformation en gris
	cvtColor(LoadedImage,gray, COLOR_BGR2GRAY);
	//appliquer la somme entre l'image et son filtre gaussien
	somme = flouterAvecAddition(gray);
	//appliquer un filtre moyenneur 
	moyenneur = MoyenneurFilter(somme);
	//reapliquer le filtre gaussien 
	gaussien = GaussianFilter(moyenneur);
	//binarisation de l'image
	bin = binarisationCV(gaussien);
	
	/*******************Etape2*********************/
	//appliquer la transformer de hough sur l'image afin d'extraire les lignes 
	lines=ligneHough(bin);
	// affichage de l'image
	//garder que les lignes horizontaux de degree max de 40
	lignesH= horizontalLines(lines,40);
	//affichage avec les lignes horizontaux 
	/*********************etape 3**********************/
	//regrouper les lignes detecter et choisir que la ligne du niveau d'eau et le bas du verre
	ligneHRegroupe = regrouperLignes(lignesH, 80);
	//affichage de l<image
	afficherImageAvecLigne(gray, ligneHRegroupe,"regroupement lignes");
	// choisir que la ligne du haut et la ligne du bas 
	ligneFinal = trierLignes(ligneHRegroupe);
	//affichage avec les lignes horizontaux 
	afficherImageAvecLigne(gray, ligneFinal,"resultat final des lignes");
	//affichage de l'image avec rectangle de la surface de l'eau
	avecRec = remplirSurface(gray, ligneFinal);
	afficher_image(avecRec, "image avec la surface de l'eau");
	waitKey(0);
}
