package sfc.models;

import javafx.event.EventHandler;
import javafx.scene.image.ImageView;
import javafx.scene.image.PixelWriter;
import javafx.scene.image.WritableImage;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.paint.Color;

/**
 * Canvas model class
 * Canvas to controll drawing on ImageView
 * @author: Filip Gulan
 * @mail: xgulan00@stud.fit.vutbr.cz
 * @date: 16.10.2017
 */
public class CanvasModel {

    public double realFrom;
    public double realTo;
    public double imFrom;
    public double imTo;
    public double zoom;
    //Attributes
    private ImageView canvas;
    private MandelbrotModel mandelbrotModel;
    private int iterations;
    private int multiplier;
    //HVS attributes
    private int hueFrom;
    private int hueTo;
    private double saturation;
    private double value;
    private Color mandelbrotPointColorPicker;
    private Thread drawThread;


    /**
     * Constructor
     * @param canvas to draw Mandelbrot
     */
    public CanvasModel(ImageView canvas) {
        this.canvas = canvas;
        this.mandelbrotModel = new MandelbrotModel(this.iterations);
    }

    /**
     * Setup colorsettings of Mandelbrot image
     * @param hueFrom hue start range
     * @param hueTo hue end range
     * @param saturation
     * @param value
     * @param multiplier multiplier for higher color diversity
     * @param mandelbrotPointColorPicker color of point that belongs to mandelbrot set
     */
    public void setColorSettings(int hueFrom, int hueTo, double saturation, double value, int multiplier, Color mandelbrotPointColorPicker) {
        this.hueFrom = hueFrom;
        this.hueTo = hueTo;
        this.saturation = saturation;
        this.value = value;
        this.multiplier = multiplier;
        this.mandelbrotPointColorPicker = mandelbrotPointColorPicker;
    }

    /**
     * Set max number of iterations of mandelbrot algorithm
     * @param iterations
     */
    public void setIterations(int iterations) {
        this.iterations = iterations;
        this.mandelbrotModel.setMaxIterations(this.iterations);
    }

    /**
     * Get real coordinate value from canvas coordinate
     * @param value coordinate in canvas
     * @return computed real coordinate
     */
    public double getReal(double value) {
        return this.realFrom + (getStepValue(this.realFrom, this.realTo, this.canvas.getFitWidth()) * value);
    }

    /**
     * Get imaginary coordinate value from canvas coordinate
     * @param value coordinate in canvas
     * @return computed imaginary coordinate
     */
    public double getIm(double value) {
        return this.imFrom + (getStepValue(this.imFrom, this.imTo, this.canvas.getFitHeight()) * value);
    }

    /**
     * Setup new render frame
     * @param X of frame
     * @param Y of frame
     * @param zoom ratio
     */
    public void setupFrame(double X, double Y, double zoom) {
        double realFromTemp = this.getReal(X - zoom);
        double realToTemp = this.getReal(X + zoom);
        double imFromTemp = this.getIm(Y - zoom);
        double imToTemp = this.getIm(Y + zoom);
        this.realFrom = realFromTemp;
        this.realTo = realToTemp;
        this.imFrom = imFromTemp;
        this.imTo = imToTemp;
    }

    /**
     * Render canvas in separated Thread
     */
    public void renderCanvas() {
        if (this.drawThread != null) {
            this.drawThread.stop();
        }
         this.drawThread = new Thread(new Runnable() {
            @Override
            public void run() {
                double imStepValue = getStepValue(imFrom, imTo, canvas.getFitWidth());
                double realStepValue = getStepValue(realFrom, realTo, canvas.getFitHeight());
                WritableImage img = new WritableImage((int)canvas.getFitWidth(), (int)canvas.getFitHeight());
                PixelWriter   pixelWriter  = img.getPixelWriter();
                int mandelbrotValue;
                double imStep = imFrom;
                double realStep = realFrom;
                for(int i = 0; i < canvas.getFitWidth(); i++) {
                    imStep = imFrom;
                    for (int j = 0; j < canvas.getFitHeight(); j++) {
                        if ((mandelbrotValue = mandelbrotModel.test(realStep, imStep)) != 0) { //it is not mandelbrot point
                            pixelWriter.setColor(i, j, Color.hsb(getHue(mandelbrotValue), saturation, value));
                        } else { //it is mandelbrot point
                            pixelWriter.setColor(i, j, mandelbrotPointColorPicker);
                        }
                        imStep += imStepValue;
                    }
                    realStep += realStepValue;
                }
                canvas.setImage(img);
            }
        });
        drawThread.start();
    }

    /**
     * Get step value/ discretisation of complex plane and get one segment
     * @param from complex part from
     * @param to complex part to
     * @param length length of drawing canvas
     * @return computed segment
     */
    private double getStepValue(double from, double to, double length) {
        if (to > 0 && from > 0 || to < 0 && from < 0) { //both values are positive or negative
            return Math.abs(Math.abs(to) - Math.abs(from)) / length;
        } else { //one value is positive one is negative
            return Math.abs(Math.abs(to) + Math.abs(from)) / length;
        }
    }

    /**
     * Get computed hue value
     * @param value initial hue
     * @return computed hue
     */
    private int getHue(int value) {
        return (this.hueTo - this.hueFrom == 0) ? this.hueTo : ((value * this.multiplier) % (Math.abs(this.hueTo - this.hueFrom))) + this.hueFrom;
    }
}
