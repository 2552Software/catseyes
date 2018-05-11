#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"

class Eye : public ofImage {
public:
    void setup() {
        load(path);
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57);
    }
private:
    const std::string path = "eye6.png"; //bugbug hard coded path... ? maybe derived classes change this?
};

// two eyes, each with its own movement
class ManagedEye {
public:
    enum MovementType { Still, Up, Down, Left, Right };

    Eye eye;
    void setup() {
        eye.setup();
        degrees = 0.0;
        maxUp = -40.0;
        maxDown = 50.0;
        incSize = 1.0;
        set(Still); // go to a known state
    }
    void update() {
        eye.update();
        degrees += inc;
        if (degrees > maxDown) {
            inc = -inc;
            degrees = maxDown;
        }
        else if (degrees <= maxUp) {
            inc = -inc;
            degrees = maxUp;
        }
    }
    void draw() {
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofPushMatrix();
        ofTranslate(eye.getWidth() / 2, eye.getHeight() / 2);//move pivot to centre
        ofRotateDeg(degrees, vecX, vecY, 0); /// 2d
        ofPushMatrix();
        ofTranslate(-eye.getWidth() / 2, -eye.getHeight() / 2, 0);//move back by the centre offset
        eye.draw(x, y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending();
        ofSetColor(255, 0, 0, 127); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }

    void set(MovementType mov, int x=0, int y=0, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        this->x = x;
        this->y = y;
        this->vecX = vecX;
        this->vecY = vecY;
        this->incSize = incSize;
        this->maxUp = maxUp;
        this->maxDown = maxDown;
        switch (mov) {
        case Still:
            inc = 0.0;
            break;
        case Up:
        case Right:
            inc = -incSize;
            break;
        case Down:
        case Left:
            inc = incSize;
            break;
        }
    }
    float degrees, vecX, vecY, maxUp, maxDown, inc, incSize;
    int x, y;
private:
    MovementType type;
};

class Camera : public ofEasyCam {
public:
    // nice drawing tool
    void drawit() {
        float time = ofGetElapsedTimef();
        float longitude = 10 * time;
        float latitude = 10 * sin(time*0.8);
        float radius = 800 + 50 * sin(time*0.4);
        orbitDeg(longitude, latitude, radius, ofPoint(0, 0, 0));
    }
    //headTrackedCamera.begin();
    //headTrackedCamera.panDeg(0.5);
    //headTrackedCamera.truck(1.0);
    //headTrackedCamera.dolly(-1.0);
    //headTrackedCamera.boom(1.0);
    //headTrackedCamera.tiltDeg(5);

};

class Contours {
public:
    void setup() {
        video.setVerbose(true);
        video.listDevices();
        video.setDeviceID(1);
        video.setup(ofGetWidth(), ofGetHeight());
        colorImg.allocate(ofGetWidth(), ofGetHeight());
        grayImage.allocate(ofGetWidth(), ofGetHeight());
        targetColor = ofColor::red;
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            thresholdImage = grayImage; // make a copy for thresholding
            thresholdImage.threshold(100); // turn any pixels above 100 white, and below 100 black
            contourFinder.findContours(thresholdImage, 5, (ofGetWidth()*ofGetHeight()) / 4, 4, false, true);
        }

        //these 2 lines of code must be called every time the head position changes
        // cam.update();
        //if (cam.isFrameNew()) {
        //     background.setLearningTime(learningTime);
        ///     background.setThresholdValue(thresholdValue);
        //     background.update(cam, thresholded);
        //     thresholded.update();
        //     blur(cam, 10);
        //     contourFinder.setTargetColor(targetColor, trackHs ? TRACK_COLOR_HS : TRACK_COLOR_RGB);
        //     contourFinder.setThreshold(threshold);
        //     contourFinder.findContours(cam);
        //}

    }
    void set(int x, int y) {
        this->x = x;
        this->y = y;
    }
    void draw() {
        ofSetColor(ofColor::white);
        if (thresholdImage.bAllocated) {
            thresholdImage.draw(x, y);
        }
        contourFinder.draw();
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            ofxCvBlob& blob = contourFinder.blobs[i];
            blob.draw(x, y);
            ofRectangle r = contourFinder.blobs.at(i).boundingRect;
            r.x += ofGetWidth(); r.y += ofGetHeight();
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c);
            ofDrawRectangle(r);
        }

    }
    ofxCvContourFinder contourFinder;
    ofColor targetColor;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage thresholdImage;
    int x, y;
};

// get all logic into one place
class Art  {
public:
    void setup() {
        countours.set(0,0);
        countours.setup();
        eye.setup();
        eye.set(ManagedEye::Up, 0, 0, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0);
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);
        ofBackground(0);

        // Set the video grabber to the ofxPS3EyeGrabber.

        ofEnableAlphaBlending();

    }
    void update() {
        countours.update();
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
        ofSetColor(255);
        eye.draw();
        countours.draw();
        cam.begin();
        // do funky stuff with Tom's art now and then
        cam.end();
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup();
    void update();
    void draw();

};


/* reference
void rotateY(ofImage &image, float degrees, int x, int y) {
ofPushMatrix();
ofTranslate(image.getWidth() / 2, image.getHeight() / 2, 0);//move pivot to centre
ofRotateYDeg(degrees);
ofPushMatrix();
ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2, 0);//move back by the centre offset
image.draw(x, y);
ofPopMatrix();
ofPopMatrix();
}

*/
