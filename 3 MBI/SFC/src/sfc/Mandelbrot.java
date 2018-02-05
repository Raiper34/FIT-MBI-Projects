package sfc;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

/**
 * Mandelbrot class - SFC project 123 Mandelbrot visualisation
 * Main class that run after program executes
 * @author: Filip Gulan
 * @mail: xgulan00@stud.fit.vutbr.cz
 * @date: 16.10.2017
 */
public class Mandelbrot extends Application {

    /**
     * Start JavaFX app
     * @param primaryStage
     * @throws Exception
     */
    @Override
    public void start(Stage primaryStage) throws Exception{
        Parent root = FXMLLoader.load(getClass().getResource("layouts/mainLayout.fxml"));
        primaryStage.setTitle("SFC Mandelbrot");
        primaryStage.setScene(new Scene(root, 800, 630));
        primaryStage.setResizable(false);
        primaryStage.show();
    }
}
