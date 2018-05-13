#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

class Motion {
public:
    float degrees; // need to store both x, y so rotation is always the same?
    glm::vec2 vec;
};

//
class ManagedEye : public ofImage {
public:
    enum MovementType { Still, Up, Down, Left, Right, Free };
    void setup() {
        load(path);
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of window
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = ofGetHeight(); // rotate around x deals with height
        max.y = ofGetWidth();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
        speed = 1; // 1 is fastest, 100 slow
        range.x = 0.0;
        range.y = 90.0;
        incSize = 1.0;
        currentDegrees.x = currentDegrees.y = currentDegrees.z = 0.0;
    }
    void update() {
        ofImage::update();
    }
    void look(MovementType mv) {
        switch (mv) {
        case Left:
            addY(min.y);
            break;
        case Right:
            addY(max.y);
            break;
        case Up:
            addX(min.x);
            break;
        case Down:
            addY(max.x);
            break;
        }
        
    }
    // make a nice path from current location (in degrees) to the target location
    void makePath(Motion motion, float currentDegrees) {
        // make nice paths
        float degrees = motion.degrees;
        if (degrees < 0) {
            for (float f = currentDegrees - incSize; f > currentDegrees + degrees; f -= incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }
        else {
            for (float f = currentDegrees + incSize; f < currentDegrees + degrees; f += incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }

    }
    void addY(float y) {
        float degreesToMove = toYDegrees(y);
        Motion motion;
        motion.vec.x = 0.0;
        motion.vec.y = 1.0;
        motion.degrees = degreesToMove;
        // make nice paths
        makePath(motion, currentDegrees.y);
        currentDegrees.y += degreesToMove;
        current.y = y;
    }
    void addX(float x) {
        float degreesToMove = toXDegrees(x);
        Motion motion;
        motion.vec.x = 1.0;
        motion.vec.y = 0.0;
        motion.degrees = degreesToMove;
        makePath(motion, currentDegrees.x);
        currentDegrees.x += degreesToMove;
        current.x = x;
    }

    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vec.x, motion.vec.y, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }

    float incSize;
    glm::vec3 currentDegrees, center, min, max, current;
    glm::vec2 range;
    int speed;
private:
    std::queue<Motion> motionQ;
    double toXDegrees(float val) {
        float delta = val - current.x;
        if (delta > 0) {
            return ofMap(delta, 0, max.x, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.x, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }
    double toYDegrees(float val) {
        float delta = val - current.y;
        if (delta > 0) {
            return ofMap(delta, 0, max.y, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.y, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }

    void pushPoint(Motion& motion) {
        for (int i = 0; i < speed; ++i) {
            motionQ.push(motion); // repeat to keep things smooth
        }
    }

    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
        set();
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (size_t i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class ElectricCat  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);
        eye.look(ManagedEye::Left);
        eye.look(ManagedEye::Right);
        eye.look(ManagedEye::Left);
        eye.look(ManagedEye::Right);
        eye.look(ManagedEye::Up);
        eye.look(ManagedEye::Down);
        eye.look(ManagedEye::Left);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
           // eye.addX(countours.blobs[0].centroid.x);
           // eye.addY(countours.blobs[0].centroid.y);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    ElectricCat art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
        if (degrees < 0) {
            for (float f = currentDegrees - incSize; f > currentDegrees + degrees; f -= incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }
        else {
            for (float f = currentDegrees + incSize; f < currentDegrees + degrees; f += incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }

    }
    void addY(float y) {
        float degrees = toYDegrees(y);
        Motion motion;
        motion.vec.x = 0.0;
        motion.vec.y = 1.0;
        motion.degrees = toYDegrees(y);
        // make nice paths
        makePath(motion, currentDegrees.y);
        currentDegrees.y = degrees;
        current.y = y;
    }
    void addX(float x) {
        float degrees = toXDegrees(x);
        Motion motion;
        motion.vec.x = 1.0;
        motion.vec.y = 0.0;
        motion.degrees = toXDegrees(x);
        makePath(motion, currentDegrees.x);
        currentDegrees.x = degrees;
        current.x = x;
    }

    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vec.x, motion.vec.y, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }

    float incSize;
    glm::vec3 currentDegrees, center, min, max, current;
    glm::vec2 range;
    int speed;
private:
    std::queue<Motion> motionQ;
    double toXDegrees(float val) {
        float delta = val - current.x;
        if (delta > 0) {
            return ofMap(delta, 0, max.x - current.x, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.x - current.x, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }
    double toYDegrees(float val) {
        float delta = val - current.y;
        if (delta > 0) {
            return ofMap(delta, 0, max.y - current.y, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.y - current.y, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }

    void pushPoint(Motion& motion) {
        for (int i = 0; i < speed; ++i) {
            motionQ.push(motion); // repeat to keep things smooth
        }
    }

    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
        set();
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (size_t i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class ElectricCat  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);
        eye.look(ManagedEye::Left);
        eye.look(ManagedEye::Right);
        eye.look(ManagedEye::Left);
        eye.look(ManagedEye::Right);
        eye.look(ManagedEye::Left);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
           // eye.addX(countours.blobs[0].centroid.x);
           // eye.addY(countours.blobs[0].centroid.y);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    ElectricCat art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
            for (float f = curentDegrees.y+ incSize; f < curentDegrees.y + degrees; f += incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }
    }
    void addX(float x) {
        float degrees = toXDegrees(x);
        Motion motion;
        motion.vec.x = 1.0;
        motion.vec.y = 0.0;
        if (degrees < 0) {
            for (float f = curentDegrees.x - incSize; f > curentDegrees.x + degrees; f -= incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }
        else {
            for (float f = curentDegrees.x + incSize; f < curentDegrees.x + degrees; f += incSize) {
                motion.degrees = f;
                pushPoint(motion);
            }
        }
    }

    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vec.x, motion.vec.y, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    float incSize;
    glm::vec3 curentDegrees;
    glm::vec3 center, min, max, current;
    glm::vec2 range;
    int speed;
private:
    std::queue<Motion> motionQ;
    double toXDegrees(float val) {
        float delta = val - current.x;
        if (delta > 0) {
            return ofMap(delta, 0, max.x - current.x, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.x - current.x, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }
    double toYDegrees(float val) {
        float delta = val - current.y;
        if (delta > 0) {
            return ofMap(delta, 0, max.y - current.y, range.x, range.y); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.y - current.y, range.x, -range.y); // center is 0, looking at the bottom is 90
        }
    }

    void pushPoint(Motion& motion) {
        for (int i = 0; i < speed; ++i) {
            motionQ.push(motion); // repeat to keep things smooth
        }
    }

    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (size_t i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);
        eye.look(ManagedEye::Left);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
           // eye.addX(countours.blobs[0].centroid.x);
           // eye.addY(countours.blobs[0].centroid.y);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
    }
    void look(MovementType mv) {
        switch (mv) {
        case Left:
            addY(0);
            break;
        }
        
    }
    void addY(float y) {
        float degrees = toYDegrees(y);
        Motion motion;
        for (float f = curentDegrees.y; f < curentDegrees.y + degrees; f += incSize) {
            motion.set(degrees, glm::vec2(0.0, 1.0));
            pushPoint(motion);
        }
    }
    void addX(float x) {
        float degrees = toXDegrees(x);
        Motion motion;
        for (float f = curentDegrees.x; f < curentDegrees.x + degrees; f += incSize) {
            motion.set(degrees, glm::vec2(1.0, 0.0));
            pushPoint(motion);
        }
    }

    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vec.x, motion.vec.y, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    void set(MovementType mov, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        type = mov;
        refresh();
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
    float maxUp, maxDown, inc, incSize;
    glm::vec3 curentDegrees;
    glm::vec3 center, min, max, current;
private:
    double toXDegrees(float val) {
        float delta = val - current.x;
        if (delta > 0) {
            return ofMap(delta, 0, max.x - current.x, 0.0, 90.0); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.x - current.x, 0.0, -90.0); // center is 0, looking at the bottom is 90
        }
    }
    double toYDegrees(float val) {
        float delta = val - current.y;
        if (delta > 0) {
            return ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
        }
        else {
            return ofMap(abs(delta), 0, max.y - current.y, 0.0, -90.0); // center is 0, looking at the bottom is 90
        }
    }

    void pushPoint(Motion& motion) {
        for (int i = 0; i < speed; ++i) {
            motionQ.push(motion); // repeat to keep things smooth
        }
    }

    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (size_t i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
           // eye.addX(countours.blobs[0].centroid.x);
           // eye.addY(countours.blobs[0].centroid.y);
          
        }
        eye.look(ManagedEye::Left);
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
            inc = -inc;
            degrees = maxUp;
        }
        */
    }
    void LookatY(glm::vec3 v) {
        float delta = v.y - current.y;
        float degrees;
        // moving down (from center to bottom is 90 degrees so clamp that do pixels)
        if (delta > 0) {
            degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            for (float f = curentDegrees.x; f < curentDegrees.x + degrees; f += incSize) {
                motionQ.push(Motion(degrees, 0.0, 1.0));
            }
        }
        else {
            degrees = ofMap(abs(delta), 0, max.y - current.y, 0.0, -90.0); // center is 0, looking at the bottom is 90
            for (float f = curentDegrees.x; f < curentDegrees.x + degrees; f += incSize) {
                motionQ.push(Motion(degrees, 0.0, 1.0));
            }
        }
        return;
    }
    // move x then y
    void LookatX(glm::vec3 v) {
        float delta = v.x - current.x;
        // moving down (from center to bottom is 90 degrees so clamp that do pixels)
        if (delta > 0) {
            float degrees = ofMap(delta, 0, max.x - current.x, 0.0, 90.0); // center is 0, looking at the bottom is 90
            motionQ.push(Motion(degrees, 1.0, 0.0));
        }
        else {
            float degrees = ofMap(abs(delta), 0, max.x - current.x, 0.0, -90.0); // center is 0, looking at the bottom is 90
            motionQ.push(Motion(degrees, 1.0, 0.0));
            return;
        }
        return;
                     // center of screen is  center.x
        if (v.y > current.y) { // looking down request
            type = Down;
            float delta = v.y - current.y;
            // moving down (from center to bottom is 90 degrees so clamp that do pixels)
            //degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            glm::vec3 newv(v);
            //newv.y = degrees;
            //locationString.push(newv);
        }
        // just doing for X first
    //    if (degrees < v.x) {
            // going forward
      //      for (float f = degrees+ incSize; f <= v.x; f += incSize) {
        //        glm::vec3 newv(v);
          //      newv.x = f;
            //    locationString.push(newv);
            //}
        //}
    }
    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vecX, motion.vecY, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    void set(MovementType mov, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        type = mov;
        refresh();
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
    float maxUp, maxDown, inc, incSize;
    glm::vec3 curentDegrees;
    glm::vec3 center, min, max, current;
private:
    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (int i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
            eye.LookatX(countours.blobs[0].centroid);
            eye.LookatY(countours.blobs[0].centroid);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
        float delta = v.y - current.y;
        float degrees;
        // moving down (from center to bottom is 90 degrees so clamp that do pixels)
        if (delta > 0) {
            degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            for (float f = curentDegrees.x; f < curentDegrees.x + degrees; f += incSize) {
                motionQ.push(Motion(degrees, 0.0, 1.0));
            }
        }
        else {
            degrees = ofMap(abs(delta), 0, max.y - current.y, 0.0, -90.0); // center is 0, looking at the bottom is 90
            for (float f = curentDegrees.x; f < curentDegrees.x + degrees; f += incSize) {
                motionQ.push(Motion(degrees, 0.0, 1.0));
            }
        }
        return;
    }
    // move x then y
    void LookatX(glm::vec3 v) {
        float delta = v.x - current.x;
        // moving down (from center to bottom is 90 degrees so clamp that do pixels)
        if (delta > 0) {
            float degrees = ofMap(delta, 0, max.x - current.x, 0.0, 90.0); // center is 0, looking at the bottom is 90
            motionQ.push(Motion(degrees, 1.0, 0.0));
        }
        else {
            float degrees = ofMap(abs(delta), 0, max.x - current.x, 0.0, -90.0); // center is 0, looking at the bottom is 90
            motionQ.push(Motion(degrees, 1.0, 0.0));
            return;
        }
        return;
                     // center of screen is  center.x
        if (v.y > current.y) { // looking down request
            type = Down;
            float delta = v.y - current.y;
            // moving down (from center to bottom is 90 degrees so clamp that do pixels)
            //degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            glm::vec3 newv(v);
            //newv.y = degrees;
            //locationString.push(newv);
        }
        // just doing for X first
    //    if (degrees < v.x) {
            // going forward
      //      for (float f = degrees+ incSize; f <= v.x; f += incSize) {
        //        glm::vec3 newv(v);
          //      newv.x = f;
            //    locationString.push(newv);
            //}
        //}
    }
    void draw() {
        if (!motionQ.empty()) {
            motion = motionQ.front();
            motionQ.pop();
        }
        //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofPushMatrix();
        ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                        //degrees = v.y;
        ofRotateDeg(motion.degrees, motion.vecX, motion.vecY, 0); /// 2d todo add in a Y
        ofPushMatrix();
        ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
        ofImage::draw(center.x, center.y);
        ofPopMatrix();
        ofPopMatrix();
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    void set(MovementType mov, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        type = mov;
        refresh();
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
    float maxUp, maxDown, inc, incSize;
    glm::vec3 curentDegrees;
    glm::vec3 center, min, max, current;
private:
    MovementType type;
    Motion motion; // save last one
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (int i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
            eye.LookatX(countours.blobs[0].centroid);
            eye.LookatY(countours.blobs[0].centroid);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
        return;
    }
    // move x then y
    void LookatX(glm::vec3 v) {
        if (--skip < 0) {
            vecX = 1.0;
            vecY = 0.0;
            float delta = v.x - current.x;
            // moving down (from center to bottom is 90 degrees so clamp that do pixels)
            if (delta > 0) {
                degrees = ofMap(delta, 0, max.x - current.x, 0.0, 90.0); // center is 0, looking at the bottom is 90
            }
            else {
                degrees = ofMap(abs(delta), 0, max.x - current.x, 0.0, -90.0); // center is 0, looking at the bottom is 90
                return;
            }
            skip = 30;
        }
        return;
                     // center of screen is  center.x
        if (v.y > current.y) { // looking down request
            type = Down;
            float delta = v.y - current.y;
            // moving down (from center to bottom is 90 degrees so clamp that do pixels)
            degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            glm::vec3 newv(v);
            newv.y = degrees;
            locationString.push(newv);
        }
        // just doing for X first
    //    if (degrees < v.x) {
            // going forward
      //      for (float f = degrees+ incSize; f <= v.x; f += incSize) {
        //        glm::vec3 newv(v);
          //      newv.x = f;
            //    locationString.push(newv);
            //}
        //}
    }
    void draw() {
        //if (!locationString.empty()) {
            //   glm::vec3 v = locationString.front();
            //    locationString.pop();
            //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
            ofSetColor(ofColor::white);
            ofSetBackgroundColor(ofColor::black);
            ofPushMatrix();
            ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
                                                         //degrees = v.y;
            ofRotateDeg(degrees, vecX, vecY, 0); /// 2d todo add in a Y
            ofPushMatrix();
            ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
            ofImage::draw(center.x, center.y);
            ofPopMatrix();
            ofPopMatrix();
            // }
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    void set(MovementType mov, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        type = mov;
        refresh();
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
    glm::vec3 center, min, max, current;
    std::queue<glm::vec3> locationString;
private:
    MovementType type;
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(60); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayDiff, 25, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (int i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);
        ofSetFrameRate(30);
        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
            eye.LookatX(countours.blobs[0].centroid);
            eye.LookatY(countours.blobs[0].centroid);
          
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
            type = Down;
            float delta = v.y - current.y;
            // moving down (from center to bottom is 90 degrees so clamp that do pixels)
            degrees = ofMap(delta, 0, max.y - current.y, 0.0, 90.0); // center is 0, looking at the bottom is 90
            glm::vec3 newv(v);
            newv.y = degrees;
            locationString.push(newv);
        }
        // just doing for X first
    //    if (degrees < v.x) {
            // going forward
      //      for (float f = degrees+ incSize; f <= v.x; f += incSize) {
        //        glm::vec3 newv(v);
          //      newv.x = f;
            //    locationString.push(newv);
            //}
        //}
    }
    void draw() {
        //if (!locationString.empty()) {
         //   glm::vec3 v = locationString.front();
        //    locationString.pop();
            //ofRotate(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
            ofSetColor(ofColor::white);
            ofSetBackgroundColor(ofColor::black);
            ofPushMatrix();
            ofTranslate(getWidth() / 2, getHeight() / 2);//move pivot to centre
            //degrees = v.y;
            ofRotateDeg(degrees, vecX, vecY, 0); /// 2d todo add in a Y
            ofPushMatrix();
            ofTranslate(-getWidth() / 2, -getHeight() / 2, 0);//move back by the centre offset
            ofImage::draw(center.x, center.y);
            ofPopMatrix();
            ofPopMatrix();
       // }
    }
    void transparentDraw() {
        ofEnableAlphaBlending(); // this would be a 50 % transparent red color
        draw();
        ofDisableAlphaBlending();
    }
    void refresh() {
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57); // size related to cat
        center.x = (ofGetWidth() / 2) - (getWidth() / 2); // middle of screen
        center.y = (ofGetHeight() / 2) - (getHeight() / 2);
        center.z = 0; // do z later
        max.x = getWidth();
        max.y = ofGetHeight();
        min.x = min.y = min.z = 0;
        current = center; // start off looking at the center
    }

    void set(MovementType mov, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        type = mov;
        refresh();
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
    glm::vec3 center, min, max, current;
    std::queue<glm::vec3> locationString;
private:
    MovementType type;
    const std::string path = "eye3.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayImage.absDiff(backgroundImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayImage.threshold(30); // turn any pixels above 30 white, and below 100 black
            contourFinder.findContours(grayImage, 5, (imgWidth*imgHeight) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        ofEnableAlphaBlending();
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        ofDisableAlphaBlending();
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        blobs.clear();
        for (int i = 0; i < contourFinder.blobs.size(); i++) {
            blobs.push_back(contourFinder.blobs[i]);
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            s += " area ", ofToString(blob.area); // blobs are sorted by size (I think) we want the largest area to focus on?
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage backgroundImage;
    std::vector<ofxCvBlob>  blobs;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);

        countours.set(0,0);
        countours.setup();
        eye.setup();
        //eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        if (countours.blobs.size() > 0) {
            eye.LookatX(countours.blobs[0].centroid);
        }
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
       // cam.begin();
        // do funky stuff with Tom's art now and then
        countours.draw();
        eye.draw();
       // cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57);
        this->x = (ofGetWidth() / 2) - (getWidth() / 2);
        this->y = (ofGetHeight() / 2) - (getHeight() / 2);
    }

    void set(MovementType mov, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        refresh();
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
    const std::string path = "eye9.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        vector<ofVideoDevice> devices= video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName == "HD USB Camera #2") {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        bUpdateBackground = true;
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        thresholdImage.allocate(imgWidth, imgHeight);
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            grayImage.blurHeavily();
            if (bUpdateBackground) {
                backgroundImage = grayImage; // save and do nothing
                bUpdateBackground = false;
            }
            else {
                grayImage.absDiff(backgroundImage);
                thresholdImage = grayImage; // make a copy for thresholding
                thresholdImage.threshold(30); // turn any pixels above 100 white, and below 100 black
                contourFinder.findContours(thresholdImage, 5, (imgWidth*imgHeight) / 4, 4, false, true);
            }
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw(ofxCvBlob& blob, float x = 0, float y = 0){
        ofPushStyle();
        ofNoFill();
        //ofSetHexColor(0x00FFFF);
        ofBeginShape();
        for (int i = 0; i < blob.nPts; i++) {
            ofVertex(x + blob.pts[i].x, y + blob.pts[i].y);
        }
        ofEndShape(true);
        //ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        ofPopStyle();
    }
    void draw() {
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        //ofSetColor(ofColor::blue, 20);
        //grayImage.draw(0, 0);
        int y = 0;
        //float imgWidth = 0.0;
        //float imgHeight = 0.0;
        ofSetLineWidth(22);
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofPoint blobCenterPnt = blob.centroid;
            std::string s = "blob ";
            s += ofToString(blobCenterPnt.x);
            s += ",";
            s += ofToString(blobCenterPnt.y);
            ofDrawBitmapString(s,0,y);
            y += 10;
            draw(blob, blobCenterPnt.x + 0 * imgWidth, blobCenterPnt.y + 2 * imgHeight);
          //  imgWidth = blob.boundingRect.width;
            //imgHeight = blob.boundingRect.height;
        }

    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage thresholdImage;
    ofxCvGrayscaleImage backgroundImage;
    int x, y;
    int imgWidth = 320; // the motion image
    int imgHeight = 240;
    bool    bUpdateBackground;

};

// get all logic into one place
class Art  {
public:
    void setup() {
        ofSetWindowShape(1000, 1000);

        countours.set(0,0);
        countours.setup();
        eye.setup();
        eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
        cam.begin();
        // do funky stuff with Tom's art now and then
        ofEnableAlphaBlending();
        countours.draw();
        //eye.draw();
        ofDisableAlphaBlending();
        cam.end();
        // put on the sun shades
    }
    Contours countours;
    ManagedEye eye;
    ofLight light;
    Camera cam;

};

class ofApp : public ofBaseApp {

public:
    Art art;
    void setup() {
        art.setup();
    }

    void update() {
        art.update();
    }

    void draw() {
        art.draw();
    }

    void keyPressed(int key) {
        if (key == 'k') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'f') {
            ofToggleFullscreen();
            art.refresh();
        }
        else if (key == 'u') {
            art.countours.bUpdateBackground = true;
        }
    }
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
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57);
        this->x = (ofGetWidth() / 2) - (getWidth() / 2);
        this->y = (ofGetHeight() / 2) - (getHeight() / 2);
    }

    void set(MovementType mov, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        refresh();
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
    const std::string path = "eye9.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        video.setVerbose(true);
        refresh();
        set();
    }
    void refresh() {
        video.resetAnchor();
        
        colorImg.allocate(ofGetWidth(), ofGetHeight());
        grayImage.allocate(ofGetWidth(), ofGetHeight());
        thresholdImage.allocate(ofGetWidth(), ofGetHeight());
    }
    void update() {
        video.update();
        //do we have a new frame?
        if (video.isFrameNew()) {
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
            grayImage.blurHeavily();
            thresholdImage = grayImage; // make a copy for thresholding
            thresholdImage.threshold(30); // turn any pixels above 100 white, and below 100 black
            contourFinder.findContours(thresholdImage, 5, (ofGetWidth()*ofGetHeight()) / 4, 4, false, true);
        }
    }
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw() {
        //if (thresholdImage.bAllocated) {
            //ofSetColor(ofColor::blue, 20);
            //thresholdImage.draw(x, y);
       // }
        // what does this do? contourFinder.draw();
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            ofxCvBlob& blob = contourFinder.blobs[i];
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
            blob.draw(blob.boundingRect.x, blob.boundingRect.y);
        }

    }
    ofxCvContourFinder contourFinder;
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
        eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0); // keep eye behind glasses
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
        cam.begin();
        // do funky stuff with Tom's art now and then
        ofEnableAlphaBlending();
        countours.draw();
        eye.draw();
        ofDisableAlphaBlending();
        cam.end();
        // put on the sun shades
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
    void keyPressed(int key);
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
        resize(ofGetWidth()*0.57, ofGetHeight()*0.57);
        this->x = (ofGetWidth() / 2) - (getWidth() / 2);
        this->y = (ofGetHeight() / 2) - (getHeight() / 2);
    }

    void set(MovementType mov, float vecX=0.0, float vecY=0.0, float incSize = 1.0, float maxUp=-40.0, float maxDown= 50.0) {
        refresh();
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
    const std::string path = "eye9.jpg"; //bugbug hard coded path... ? maybe derived classes change this?
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
        targetColor = ofColor::red;
        refresh();
        set();
    }
    void refresh() {
        video.setup(ofGetWidth(), ofGetHeight());
        colorImg.allocate(ofGetWidth(), ofGetHeight());
        grayImage.allocate(ofGetWidth(), ofGetHeight());
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
    void set(int x=0, int y=0) {
        this->x = x;
        this->y = y;
    }
    void draw() {
        if (thresholdImage.bAllocated) {
            ofSetColor(ofColor::blue, 100);
            thresholdImage.draw(x, y);
        }
        ofSetColor(ofColor::orangeRed, 100);
        contourFinder.draw();
        ofFill();
        for (int i = 0; i < contourFinder.nBlobs; i++) {
            ofxCvBlob& blob = contourFinder.blobs[i];
            blob.draw(x, y);
            ofRectangle r = contourFinder.blobs.at(i).boundingRect;
            r.x += 100; r.y += 100;
            ofColor c;
            c.setHsb(i * 64, 255, 255);
            ofSetColor(c, 100);
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
        eye.set(ManagedEye::Up, 1.0, 0.0); //bugbug size 1000 etc needs to be encapuslated
        cam.setPosition(0, 0, 0);
        light.enable();
        light.setPosition(ofVec3f(000, 000, 2000));
        light.lookAt(ofVec3f(0, 0, 0));
        ofEnableLighting();
        ofEnableSmoothing();
        ofSetVerticalSync(true);

        // Set the video grabber to the ofxPS3EyeGrabber.

    }
    void refresh() {
        countours.refresh();
        eye.refresh();
    }
    void update() {
        countours.update();
        eye.update();
    }
    void draw() {
        ofEnableDepthTest();
        eye.draw();
        ofEnableAlphaBlending();
        countours.draw();
        ofDisableAlphaBlending();
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
    void keyPressed(int key);
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
