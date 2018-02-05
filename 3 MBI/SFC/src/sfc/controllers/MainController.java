package sfc.controllers;

import javafx.embed.swing.SwingFXUtils;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.canvas.Canvas;
import javafx.scene.control.ColorPicker;
import javafx.scene.control.Label;
import javafx.scene.control.MenuBar;
import javafx.scene.control.Slider;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.AnchorPane;
import javafx.scene.paint.Color;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import sfc.models.CanvasModel;
import javax.imageio.ImageIO;
import java.awt.image.RenderedImage;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;

/**
 * Main controller class
 * Main controller for mainLayout.xml
 * @author: Filip Gulan
 * @mail: xgulan00@stud.fit.vutbr.cz
 * @date: 16.10.2017
 */
public class MainController implements Initializable {

    @FXML
    private Canvas spectrumCanvas;

    @FXML
    MenuBar menuBar;

    @FXML
    Slider hueFromSlider;

    @FXML
    Slider hueToSlider;

    @FXML
    Slider saturationSlider;

    @FXML
    Slider valueSlider;

    @FXML
    Slider iterationSlider;

    @FXML
    Slider multiplierSlider;

    @FXML
    Label canvasXLabel;

    @FXML
    Label canvasYLabel;

    @FXML
    Label mandelbrotXLabel;

    @FXML
    Label mandelbrotYLabel;

    @FXML
    Label leftTopCornerLabel;

    @FXML
    Label rightBottomCornerLabel;

    @FXML
    Label zoomLabel;

    @FXML
    ColorPicker mandelbrotPointColorPicker;

    @FXML
    AnchorPane aboutPane;

    @FXML
    private ImageView image;

    private CanvasModel canvasModel;

    /**
     * Initialize JavaFx component
     * @param url
     * @param rb
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        this.hideAbout();
        this.initializeSpectrumCanvas();
        this.canvasModel = new CanvasModel(this.image);
        this.resetCanvas();
        this.image.setOnMouseMoved(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent mouse) {
                canvasXLabel.setText("Canvas X: " + mouse.getX());
                canvasYLabel.setText("Canvas Y: " + mouse.getY());
                mandelbrotXLabel.setText("Real: " + canvasModel.getReal(mouse.getX()));
                mandelbrotYLabel.setText("Im: " + canvasModel.getIm(mouse.getY()));
                leftTopCornerLabel.setText(canvasModel.realFrom + " , " + canvasModel.imTo);
                rightBottomCornerLabel.setText(canvasModel.realTo + " , " + canvasModel.imFrom);
                //zoomLabel.setText(canvasModel.zoom + "X");
            }
        });
        this.image.setOnScroll(new EventHandler<ScrollEvent>() {
            @Override
            public void handle(ScrollEvent event) {
                double zoom = event.getDeltaY() < 0 ? image.getFitWidth() * 1/3 : image.getFitWidth() * 2/3;
                canvasModel.setupFrame(image.getFitWidth()/2, image.getFitHeight()/2, zoom);
                canvasModel.renderCanvas();
                leftTopCornerLabel.setText(canvasModel.realFrom + " , " + canvasModel.imTo);
                rightBottomCornerLabel.setText(canvasModel.realTo + " , " + canvasModel.imFrom);
            }
        });
        this.image.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent mouse) {
                canvasModel.setupFrame(mouse.getX(), mouse.getY(), image.getFitWidth() / 2);
                canvasModel.renderCanvas();
                leftTopCornerLabel.setText(canvasModel.realFrom + " , " + canvasModel.imTo);
                rightBottomCornerLabel.setText(canvasModel.realTo + " , " + canvasModel.imFrom);
            }
        });
    }

    /**
     * Initialize spectrum canvas
     */
    private void initializeSpectrumCanvas() {
        for (int i = 0; i < this.spectrumCanvas.getWidth(); i++) {
            this.spectrumCanvas.getGraphicsContext2D().setFill(Color.hsb(256 / 100 * i, 1.0, 1.0));
            this.spectrumCanvas.getGraphicsContext2D().fillRect(i, 0,1, this.spectrumCanvas.getHeight());
        }
    }

    /**
     * Save Mandelbrot image to file
     */
    public void saveCanvasToImage() {
        FileChooser fileChooser = new FileChooser();
        FileChooser.ExtensionFilter extFilter = new FileChooser.ExtensionFilter("PNG files", "*.png");
        fileChooser.getExtensionFilters().add(extFilter);
        File file = fileChooser.showSaveDialog(spectrumCanvas.getScene().getWindow());

        if (file != null) {
            try {
                file = new File(file.getParentFile(), file.getName() + ".png");
                RenderedImage renderedImage = SwingFXUtils.fromFXImage(image.getImage(), null);
                ImageIO.write(renderedImage, "png", file);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Close application
     */
    public void closeClick() {
        final Stage stage = (Stage) this.menuBar.getScene().getWindow();
        stage.close();
    }

    /**
     * Reset canvas image
     */
    public void resetCanvas() {
        this.hueFromSlider.setValue(0.0);
        this.hueToSlider.setValue(360.0);
        this.saturationSlider.setValue(1.0);
        this.valueSlider.setValue(1.0);
        this.multiplierSlider.setValue(5.0);
        this.iterationSlider.setValue(100.0);
        this.mandelbrotPointColorPicker.setValue(Color.BLACK);
        this.canvasModel.setColorSettings(
                (int)this.hueFromSlider.getValue(),
                (int)this.hueToSlider.getValue(),
                this.saturationSlider.getValue(),
                this.valueSlider.getValue(),
                (int)this.multiplierSlider.getValue(),
                this.mandelbrotPointColorPicker.getValue()
        );
        this.canvasModel.setIterations((int)this.iterationSlider.getValue());
        this.canvasModel.realFrom = -2.0;
        this.canvasModel.realTo = 2.0;
        this.canvasModel.imFrom = -2.0;
        this.canvasModel.imTo = 2.0;
        this.canvasModel.zoom = 1.0;
        this.canvasModel.renderCanvas();
    }

    /**
     * Set color settings to mandelbrot image
     */
    public void setColorSetting() {
        this.canvasModel.setColorSettings(
                (int)this.hueFromSlider.getValue(),
                (int)this.hueToSlider.getValue(),
                this.saturationSlider.getValue(),
                this.valueSlider.getValue(),
                (int)this.multiplierSlider.getValue(),
                this.mandelbrotPointColorPicker.getValue()
        );
        this.canvasModel.renderCanvas();
    }

    /**
     * Show about modal
     */
    public void showAbout() {
        this.aboutPane.setVisible(true);
    }

    /**
     * Hide about modal
     */
    public void hideAbout() {
        this.aboutPane.setVisible(false);
    }

    /**
     * Set max iterations for mandelbrot test algorithm
     */
    public void setIterations() {
        this.canvasModel.setIterations((int)this.iterationSlider.getValue());
        this.canvasModel.renderCanvas();
    }
}
