#include<stdio.h>

double integrate(double (*p)(double), double a, double b){
    double width = 0.001;
    double area = 0.0;
    for(double x=a; x<b; x+=width){
        area += p(x)*width;
    }
    return area;
}

double square(double x){
    return x*x;
}

int main(void){
    printf("integrate( square, 0.0, 2.0) = %f\n", integrate( square, 0.0, 2.0));
}