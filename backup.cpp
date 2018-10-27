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
    void draw() {
        ofPoint point = getPoint();
        // always draw at the last point to keep path connected bugbug after a few minutes go back to home
        ofLogNotice() << "rotate " << point;
        ofRotateXDeg(point.x);
        ofRotateYDeg(point.y);
        ofRotateZDeg(point.z);
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


    }
    // convert point on X to a rotation
    float turnToX(float x) {
        // 0 is no turn, - is turn right, + left
        return ofMap(x, 0, ofGetHeight(), -45, 45);
    }
    float turnToY(float y) {
        // 0 is no turn, - is turn right, + left
        return ofMap(y, 0, ofGetWidth(), 45, -45);
    }
    ofVec3f currentRotation;
    void update() {
        ofVec3f nextRotation;
        float f = 1.0f / fps;
        animator.update(f);
        color.update(f);
        camera.update(f);
        path.update(f);
        if (contours.update()) {
            int max = 0;
            ofxCvBlob blob2;
            for (auto& blob : contours.contourFinder.blobs) {
                if (blob.area > max) {
                    max = blob.area;
                    blob2 = blob;
                }
            }
            // just add the first one?  or all of them?
            float x = (ofGetWidth() / imgWidth)*(imgWidth - blob2.centroid.x);
            float y = (ofGetHeight() / imgHeight)*blob2.centroid.y;
            nextRotation.x = x;
            nextRotation.y = y;
            path.append(ofPoint(turnToX(x), turnToY(y)));

        }
        for (ofImage& image : images) {
            image.update(); // keep updated
        }
        if (nextRotation.x != currentRotation.x || nextRotation.y != currentRotation.y || nextRotation.z != currentRotation.z) {
            currentRotation = nextRotation;
            sphere.rotateDeg(nextRotation.x, 1.0f, 0.0f, 0.0f);
            sphere.rotateDeg(nextRotation.y, 0.0f, 1.0f, 0.0f);
            sphere.rotateDeg(nextRotation.z, 0.0f, 0.0f, 1.0f);
            freshSphere = false;
        }
        else if (!freshSphere) {
            currentRotation.x = currentRotation.y = currentRotation.z = 0.0f;
            sphere = ofSpherePrimitive(); // reset
            sphere.rotateDeg(180.0f, 0.0f, 1.0f, 0.0f); // flip to front
            sphere.setResolution(25);
            sphere.setRadius(std::min(ofGetWidth(), ofGetHeight()));
            freshSphere = true;
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
        contours.draw();
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
    ofSpherePrimitive sphere;

private:
    std::vector<ofImage> images;
    bool freshSphere = false;
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
