package sfc.models;

/**
 * Mandelbrot model class
 * Class to test mandelbrot point
 * @author: Filip Gulan
 * @mail: xgulan00@stud.fit.vutbr.cz
 * @date: 16.10.2017
 */
class MandelbrotModel {

    private int maxIterations;

    /**
     * Constructor
     * @param maxIterations
     */
    MandelbrotModel(int maxIterations) {
        this.maxIterations = maxIterations;
    }

    /**
     * Set up maxiterations/precission
     * @param maxIterations
     */
    void setMaxIterations(int maxIterations) {
        this.maxIterations = maxIterations;
    }

    /**
     * Test if point belongs to Mandelbrot or not
     * @param cReal real part of complex number
     * @param cIm imaginary part of complex number
     * @return number of iterations
     */
    int test(double cReal, double cIm) {
        double zReal = 0.0;
        double zIm = 0.0;
        double zRealTemp;
        double zImTemp;
        int i = 0;

        do {
            zImTemp = zIm * zIm;
            zRealTemp = zReal * zReal;
            zIm = 2.0 * zReal * zIm + cIm;
            zReal = zRealTemp - zImTemp + cReal;
            i++;
            if (zRealTemp + zImTemp >= 4.0) { //it is not mandelbrot point
                return i;
            }
        } while (i <= this.maxIterations);
        return 0; //it is mandelbrot point
    }
}
