#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

// debug helper
//std::stringstream ss;
//ss << eyeAnimator.getIndex();
//ofSetWindowTitle(ss.str());

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
        clearQueue();
    }
    void addTransition(const ofxAnimatableOfPoint targetValue) {
        animSteps.push_back(targetValue);
    }
    void clearQueue() {
        animSteps.clear();
    }
    ofxAnimatableOfPoint getCurrentValue() {
        return anim;
    }

    void update(float dt) {
        if (playing) {
            anim.update(dt);
            if (anim.hasFinishedAnimating()) {
                if (currentStep < animSteps.size()) {
                    //animSteps.erase(animSteps.begin());
                    anim = animSteps[currentStep];
                    currentStep++;
                }
                else {
                    clearQueue();
                    currentStep = 0;
                    playing = false; // stop when empty, restart when one added
                }
            }
        }
    }

    void draw() {
        if (playing) {
            ofPoint p = getCurrentValue().getCurrentPosition();
            ofRotateXDeg(p.x);
            ofRotateYDeg(p.y);
            ofRotateZDeg(p.z);
        }
    }
    void append(const ofPoint& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(animSteps[animSteps.size() - 1].getCurrentPosition());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.5);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(QUADRATIC_EASE_OUT);
        addTransition(targetPoint);
        if (!playing) {
            startPlaying();
        }
    }

    void startPlaying() {
        currentStep = 0;
        playing = true;
    }

    void pausePlayback() {
        playing = false;
    }

    void resumePlayback() {
        playing = true;
    }

protected:
    size_t currentStep = 0;
    bool playing = false;
private:
    ofxAnimatableOfPoint anim;
    std::vector<ofxAnimatableOfPoint> animSteps;
    bool infinite = false;
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
        }
        return false; // no update
    }
    void draw() {
        ofPushStyle();
        ofPushMatrix();
        ofNoFill();
        ofSetLineWidth(10);// ofRandom(1, 5));
        for (auto& blob : contourFinder.blobs) {
            ofPolyline line;
            for (int i = 0; i < blob.nPts; i++) {
                float x = (ofGetWidth() / imgWidth)*(imgWidth - blob.pts[i].x);
                float y = (ofGetHeight() / imgHeight)*blob.pts[i].y;
                // ofLogNotice() << "point x " << x << " point y " << y;
                line.addVertex(x, y);
            }
            line.close();
            line.draw();
            //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
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
    void setup() {
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


    }
    // convert point on X to a rotation
    float turnX(float x) {
        // 0 is no turn, - is turn right, + left
        return ofMap(x, 0, ofGetWidth(), -70, 70);
    }
    float turnY(float y) {
        // 0 is no turn, - is turn right, + left
        return ofMap(y, 0, ofGetHeight(), 45, -45);
    }
    void update(float f) {
        animator.update(f);
        color.update(f);
        camera.update(f);
        path.update(f);
        if (contours.update()) {
            // just add the first one?  or all of them?
            float x = (ofGetWidth() / imgWidth)*(imgWidth - contours.contourFinder.blobs[0].pts[0].x);
            float y = (ofGetHeight() / imgHeight)*contours.contourFinder.blobs[0].pts[0].y;
            path.append(ofPoint(turnX(x), turnY(y)));

        }
        for (ofImage& image : images) {
            image.update(); // keep updated
        }
    }
    void draw(float r) {
        bind();
        ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
        ofRotateXDeg(180.0f); // hide seam
        contours.draw();
        ofDrawSphere(0.0f, 0.0f, 0.0f, r);
        unbind();
        path.draw();
    }
    void startPlaying() {
       setup();
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
        ofRotateZDeg(180.0);
        getImage().getTexture().bind();
    }
    ContoursBuilder contours;
    ofxAnimatableFloat camera;
    ofxAnimatableFloat animator; 
    ofxAnimatableOfColor color; // image  colors
    ofxAnimatableQueueOfPoint path; // path of image

private:
    std::vector<ofImage> images;
};

class ofApp : public ofBaseApp{

	public:
        const float fps = 30.0f;
        float r;
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
    ofSetFrameRate(fps);
    ofBackground(ofColor::black);
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofDisableArbTex();
    ofSetSmoothLighting(true);
    //ofDisableAlphaBlending();

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

    // see size handler too
    windowResized(0,0);

}

//--------------------------------------------------------------
void ofApp::update(){
    eyeAnimator.update(1.0f / fps);
    //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
    light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
    // debug helper
    std::stringstream ss;
    //ss << countours.point.getCurrentPosition();
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
    eyeAnimator.draw(r);
    camera.end();
    material.end();
    light.disable();
    eyeAnimator.contours.draw();
    ofPopStyle();
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

    float size = std::min(ofGetHeight(), ofGetHeight());
    r = std::min(ofGetHeight(), ofGetHeight()) / 3;
    eyeAnimator.camera.reset(ofGetHeight());
    eyeAnimator.camera.animateTo(ofGetHeight() / 2);
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









