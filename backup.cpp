#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;
const int cxScreen = 2880; // screen size mac 2880x1800 pc  3840 x 2160
const int cyScreen = 1800;

// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    void draw() {
        ofPushMatrix();
        ofTranslate((ofGetWidth() / 2) - getRadius(), ofGetHeight() / 2, 0);
        ofSpherePrimitive::draw();
        ofPopMatrix();
    }
    void rotateTo(ofVec3f target) {
        // current x is 180, target is 5, need to move to 5 but -175
        // current x is 5, target is 180, need to move to 180 but +175
        currentRotation = target - currentRotation;
        ofLogNotice() << "currentRotation " << currentRotation;
        rotate(currentRotation);
        currentRotation = target;
    }
    void home() {
        rotateTo(ofVec3f(0.0f, 0.0f));
    }
private:
    void rotate(ofVec3f target) { 
        rotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        rotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        rotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
};




// debug helper
//std::stringstream ss;
//ss << eyeAnimator.getIndex();
//ofSetWindowTitle(ss.str());

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 1) {
            ofLogNotice() << " cap list size to 3 ";
            animSteps.pop_back(); // only keep the  most recent
        }
        animSteps.push_back(targetValue);
    }
    ofxAnimatableOfPoint getCurrentValue() {
        return anim;
    }

    void update(float dt) {
        if (playing) {
            anim.update(dt);
            if (anim.hasFinishedAnimating()) {
                if (animSteps.size() > 0) {
                    //animSteps.erase(animSteps.begin());
                    anim = animSteps.front();
                    animSteps.pop_front();
                    ofLogNotice() << "next";

                }
                else {
                    anim.setPosition(ofPoint()); // 0, 0, 0 will not rotate
                    playing = false; // stop when empty, restart when one added
                    ofLogNotice() << "stop";
                }
            }
        }
    }
    ofPoint getPoint() {
        ofLogNotice() << "percent done " << anim.getPercentDone();
        return anim.getCurrentPosition();
    }
    void append(const ofPoint& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.15);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
        if (!playing) {
            startPlaying();
        }
    }

    void startPlaying() {
        playing = true;
    }


protected:
    bool playing = false;
private:
    ofxAnimatableOfPoint anim;
    std::list<ofxAnimatableOfPoint> animSteps;
};


class ContoursBuilder {
public:
    void setup() {
        vector<ofVideoDevice> devices = video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName.find("facetime") == std::string::npos) {
                video.setDeviceID(device.id);
                break;
            }
        }
        video.setVerbose(true);
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);
    }
    // return true if updated
    bool update() {
        video.update();
        if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                  // clear less often
            colorImg.setFromPixels(video.getPixels());
            grayImage = colorImg; // convert our color image to a grayscale image
                                  //grayImage.blurHeavily();
            if (backgroundImage.bAllocated) {
                grayDiff.absDiff(backgroundImage, grayImage);
            }
            backgroundImage = grayImage; // only track new items -- so eye moves when objects move
            grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
            if (contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true) > 0) {
                return true;
            }
            else {
                contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
            }
        }
        return false; // no update
    }
    void draw() {
        ofPushStyle();
        ofPushMatrix();
        float w = ofGetWidth();
        float xFactor = (ofGetWidth() / imgWidth);
        float yFactor = (ofGetHeight() / imgHeight);
        //ofTranslate(0, ofGetHeight()/4, 0);
        ofNoFill();
        ofSetLineWidth(10);// ofRandom(1, 5));
        for (auto& blob : contourFinder.blobs) {
            ofPolyline line;
            for (int i = 0; i < blob.nPts; i++) {
                line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
            }
            line.close();
            line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
            line.draw();
            //ofDrawRectangle(blob.boundingRect.x, blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
        }
        ofPopMatrix();
        ofPopStyle();
    }

    ofxCvContourFinder contourFinder;
private:
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
};


class ImageAnimator {
public:        
    const float fps = 60.0f;
    
    void setup() {
        ofSetFrameRate(fps);

        animator.reset(0.0f);
        animator.setDuration(2.0f);
        animator.setRepeatType(LOOP);
        animator.setCurve(LINEAR);

        camera.reset(500.0f);
        camera.setDuration(5.0f);
        camera.setRepeatType(LOOP_BACK_AND_FORTH);
        camera.setCurve(EASE_IN);

        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);

        contours.setup();
        startPlaying();

        sphere.rotateDeg(180.0f, 0.0f, 1.0f, 0.0f); // flip to front
        sphere.setResolution(25);
        sphere.setRadius(std::min(ofGetWidth(), ofGetHeight()));
    }
    // convert point on X to a rotation
    float turnToX(float x) {
        return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
    }
    float turnToY(float y) {
        return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
    }
    void update() {
        float f = 1.0f / fps;
        animator.update(f);
        color.update(f);
        camera.update(f);
        path.update(f);
        float y = 0.0f; //(ofGetHeight() / imgHeight)
        float x = 0.0f;
        float max = 0.0f;
        if (contours.update()) {
            for (auto& blob : contours.contourFinder.blobs) {
                if (blob.area > max && blob.centroid.x > 1 && blob.centroid.y > 1) {  //x,y 1,1 is some sort of strange case
                    y = blob.centroid.x; //(ofGetHeight() / imgHeight)
                    x = blob.centroid.y;
                }
            }
            // just add the first one?  or all of them?
            // rotate is the opposite
            sphere.rotateTo(ofVec3f(turnToX(x), turnToY(y)));
            holdPosition = fps;
            path.append(ofPoint(turnToX(x), turnToY(y)));
        }
        else {
            if (--holdPosition < 0){
                sphere.home();
            }
        }
        for (ofImage& image : images) {
            image.update(); // keep updated
        }
    }
    void scale() {
        //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    }
    void windowResized(int w, int h) {
        sphere.setRadius(std::min(w, h));
        //no camera animation at this time. Please stay tuned camera.reset(h);
        //camera.animateTo(h / 2);
    }
    void draw() {
        //vector<ofVec3f> spherePoints = sphere.getMesh().getVertices();

        bind();
        scale();
        sphere.draw();
        unbind();
    }
    void startPlaying() {
       string path = "";
       ofDirectory dir(path);
       dir.allowExt("png");
       dir.allowExt("jpg");
       dir.listDir();
       for (size_t i = 0; i < dir.size(); i++) {
           add(dir.getPath(i));
       }

       animator.animateTo(images.size()); 
    }
    void add(const std::string &name) {
        ofImage image;
        if (image.load(name)){
            images.push_back(image);
        }
    }
    void add(ofImage image) {
        images.push_back(image);
    }
    ofImage& getImage() {
        return images[getIndex()];
    }
    int getIndex() {
        return (int)animator.getCurrentValue();
    }
    // allow for various eyes
    void unbind() {
        getImage().getTexture().unbind();
    }
    void bind() {
        color.applyCurrentColor();
        //ofRotateZDeg(180.0);
        getImage().getTexture().bind();
    }
    ContoursBuilder contours;
    ofxAnimatableFloat camera;
    ofxAnimatableFloat animator; 
    ofxAnimatableOfColor color; // image  colors
    ofxAnimatableQueueOfPoint path; // path of image
    SuperSphere sphere; //ofSpherePrimitive

private:
    std::vector<ofImage> images;
    float holdPosition = fps;
};

class ofApp : public ofBaseApp{

	public:
        ofLight	light;
        ofEasyCam camera;
        ImageAnimator eyeAnimator;
        ofMaterial material;
		void setup();
		void update();
		void draw();
        void setLightOrientation(ofVec3f rot)       {
            ofVec3f xax(1, 0, 0);
            ofVec3f yax(0, 1, 0);
            ofVec3f zax(0, 0, 1);
            ofQuaternion q;
            q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
            light.setOrientation(q);
        }
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

private:
};

#include "ofApp.h"

//
//--------------------------------------------------------------
void ofApp::setup(){
    ofSetLogLevel(OF_LOG_NOTICE);
    ofLogToConsole();
    ofBackground(ofColor::black);
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofDisableArbTex();
    ofSetSmoothLighting(true);
    ofDisableAlphaBlending();
    camera.setDistance(4286); // magic number 
    eyeAnimator.setup();
    light.setup();
    light.setDiffuseColor(ofFloatColor(255.0, 0.0, 0.0f));
    light.setSpecularColor(ofColor(0, 0, 255));
    light.setDirectional();
    setLightOrientation(ofVec3f(0.0f, -80.0f, 00.0f));
    material.setShininess(120);
    material.setSpecularColor(ofColor::yellow);
    material.setEmissiveColor(ofColor::blue);
    material.setDiffuseColor(ofColor::sandyBrown);
    material.setAmbientColor(ofColor::white);

}

//--------------------------------------------------------------
void ofApp::update(){
    eyeAnimator.update();
    //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
   // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
    // debug helper
    std::stringstream ss;
    ss << camera.getDistance();
    ofSetWindowTitle(ss.str());
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofPushStyle();
    light.enable();
    material.begin();
    camera.begin();
    eyeAnimator.draw();
    camera.end();
    material.end();
    light.disable();
    eyeAnimator.contours.draw();
    ofPopStyle();
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    eyeAnimator.windowResized(w, h);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
