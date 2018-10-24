##pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

class MotionData {
public:
    MotionData() {}
    MotionData(glm::vec2&r, ofRectangle &rec, int index = 0) {
        rotation = r;
        rect = rec;
        this->index = index;
    }
    void draw(int scaleX, int scaleY, bool fill = false) const {
        ofRectangle rectToScale = rect;
        rectToScale.scale(scaleX, scaleY);
        if (fill) {
            ofFill();
        }
        else {
            ofNoFill();
        }
        ofDrawRectangle(rectToScale.x*scaleX, rectToScale.y*scaleY, rectToScale.width, rectToScale.height);
    }
    bool found(int x, int y) {
        return rect.inside(x, y);
    }
    glm::vec2 rotation;
    ofRectangle rect; // target
    int index = 0;
};
class Timer {
public:
    uint64_t t1 = 0;
    uint64_t duration = 60; // 2 sec or so
    void start() {
        t1 = ofGetFrameNum();
    }
    bool current() {
        return duration > ofGetFrameNum() - t1;
    }
};
class myBlob : public ofxCvBlob {
public:
    void set(ofxCvBlob& b) {
        blob = b;
        t.duration = 5;
        t.start();
    }
    Timer t;
    ofxCvBlob blob;
};
class Contours {
public:
    std::vector<MotionData> motionMap;

    const MotionData& find(int x, int y) {
        for (auto& motion : motionMap) {
            if (motion.found(x, y)) {
                return motion;
            }
        }
        return motionMap[0];// default to empty data, no movement needed
    }
    //v.x - motion x, v.y motion y, v.w - width, v.z - length
    void add(const ofRectangle& grid, int index) { // private
        float w = grid.getWidth();
        float h = grid.getHeight();
        float row = motionMap.size() / 4;
        motionMap.push_back(MotionData(glm::vec2(grid.getX(), grid.getY()), ofRectangle(w * 0, row*h, w, h), index)); //ex: row 1, col 1
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 5.0, grid.getY()), ofRectangle(w * 1, row*h, w, h), index + 1));// row 1, col 2
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 7.0, grid.getY()), ofRectangle(w * 2, row*h, w, h), index + 2)), index + 2;// row 1, col 3
        motionMap.push_back(MotionData(glm::vec2(grid.getX() + 9.0, grid.getY()), ofRectangle(w * 3, row*h, w, h), index + 3));// row 1, col 4
    }
    void setup() {
        //video.setVerbose(true);
        vector<ofVideoDevice> devices = video.listDevices();
        for (auto& device : devices) {
            if (device.deviceName.find("FaceTime") == std::string::npos) {
                video.setDeviceID(device.id);
                break;
            }
        }
        float w = imgWidth / 4;
        float h = imgHeight / 4;
        // 0-360
        add(ofRectangle(-12.0, 10.0, w, h), 0); // col 0, % of 180
        add(ofRectangle(-10.0, 5.0, w, h), 4); // col 1
        add(ofRectangle(10, -7.0, w, h), 8); // col 2
        add(ofRectangle(12, -10.0, w, h), 12); // col 3

        video.setVerbose(true);
        video.setup(imgWidth, imgHeight);
        colorImg.allocate(imgWidth, imgHeight);
        grayImage.allocate(imgWidth, imgHeight);
        grayDiff.allocate(imgWidth, imgHeight);

    }
    void update() {
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
            contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true);
        }
    }
    void draw(const ofxCvBlob& blob) {
        ofPolyline line;
        //video.draw(video.getWidth(),0,-video.getWidth(),video.getHeight());
        for (int i = 0; i < blob.nPts; i++) {
            line.addVertex(ofPoint(imgWidth - blob.pts[i].x, blob.pts[i].y));
            //ofVertex(blob.pts[i].x, blob.pts[i].y);
        }
        line.close();
        //line.scale(cx / imgWidth, cy / imgHeight);
        line.draw();
        ofSetHexColor(0xff0099);
        //ofDrawRectangle(x + blob.boundingRect.x, y + blob.boundingRect.y, blob.boundingRect.width, blob.boundingRect.height);
    }
    void draw() {
        ofPushStyle();
        ofNoFill();
        ofSetLineWidth(1);// ofRandom(1, 5));
        bool first = true;
        for (auto& blob : contourFinder.blobs) {
            if (first) {
                ofNoFill();
                // for (auto& motion : motionMap) {
                ///     motion.draw(cx / imgWidth, cy / imgHeight);
                //}
                MotionData data = find(imgWidth - blob.centroid.x, blob.centroid.y);
                first = false; // largest blob is first
            }
            myBlob b;
            b.set(blob);
            myBlobs.push_back(b);
        }
        if (contourFinder.blobs.size() == 0) {
        }
        ofSetColor(ofColor(ofRandom(100, 255), ofRandom(100, 255), ofRandom(100, 255)));
        for (auto blob = myBlobs.begin(); blob != myBlobs.end(); ) {
            if ((*blob).t.current()) {
                //light.setAmbientColor(ofColor(ofRandom(255), ofRandom(255), ofRandom(255)));
                draw((*blob).blob);
                ++blob;
            }
            else {
                blob = myBlobs.erase(blob);
            }
        }

        ofPopStyle();
    }
    ofxCvContourFinder contourFinder;
    ofVideoGrabber video;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    std::vector<myBlob> myBlobs;
    const int imgWidth = 320;// 320; // the motion image from the camera
    const int imgHeight = 240;//240;
};

// get all logic into one place
class ElectricCat {
public:
    void setup() {
        ofSetColor(ofColor::white);
        ofSetBackgroundColor(ofColor::black);
        ofSetFrameRate(30);
        countours.setup();
        ofSetVerticalSync(true);
        // Set the video grabber to the ofxPS3EyeGrabber.
    }
    void update() {
        countours.update();
    }
    void draw() {
        ofEnableDepthTest();
        countours.draw();
        //cam.end();
        // put on the sun shades
    }
    Contours countours;

};

class ofxAnimatableQueueOfPoint {

public:
    ofxAnimatableQueueOfPoint() {
        clearQueue();
    }

    struct EventArg {
        ofxAnimatableQueueOfPoint* who;
    };

    void ofxAnimatableQueueOfPoint::setInitialValue(const ofxAnimatableOfPoint val) {
        anim = val;
    }

    void ofxAnimatableQueueOfPoint::addTransition(const ofxAnimatableOfPoint targetValue) {
        animSteps.push_back(targetValue);
    }

    void ofxAnimatableQueueOfPoint::clearQueue() {
        animSteps.clear();
    }
    ofxAnimatableOfPoint ofxAnimatableQueueOfPoint::getCurrentValue() {
        return anim;
    }

    void ofxAnimatableQueueOfPoint::update(float dt) {

        if (playing) {

            anim.update(dt);

            if (anim.hasFinishedAnimating()) {
                if (currentStep < animSteps.size()) {
                    //animSteps.erase(animSteps.begin());
                    anim = animSteps[currentStep];
                    currentStep++;
                }
                else {
                    if (infinite) {
                        currentStep = 0;
                        anim = animSteps[currentStep];
                    }
                    else {
                        playing = false;
                    }
                    EventArg arg = { this };
                    ofNotifyEvent(eventQueueDone, arg);
                }
            }
        }
    }

    ofEvent<ofxAnimatableQueueOfPoint::EventArg> eventQueueDone;

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
    bool infinite = true;
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
    void update(float f) {
        animator.update(f);
        color.update(f);
        camera.update(f);
        for (ofImage& image : images) {
            image.update(); // keep updated
        }
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
    ofxAnimatableFloat camera;
    ofxAnimatableFloat animator; 
    ofxAnimatableOfColor color; // image background color
private:
    std::vector<ofImage> images;
};

class MotionGenerator {
public:
    void draw() {
        ofPoint p = queue.getCurrentValue().getCurrentPosition();
        ofRotateXDeg(p.x);
        ofRotateYDeg(p.y);
        ofRotateZDeg(p.z);
    }
    void add(const ofPoint& start, const ofPoint& target) {
        ofxAnimatableOfPoint targetPoint;
        targetPoint.setPosition(start);
        targetPoint.animateTo(target);
        targetPoint.setDuration(2.3);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(QUADRATIC_EASE_OUT);
        queue.addTransition(targetPoint);
    }
    void update(float f) {
        queue.update(f);
    }
    void startPlaying() {
        queue.startPlaying();
    }

    ofxAnimatableQueueOfPoint queue;
};

class ofApp : public ofBaseApp{

	public:
        const float fps = 60.0f;
        float r;
        ofLight	light;
        ofEasyCam camera;
        MotionGenerator motion;
        ImageAnimator eyeAnimator;
        ofMaterial material;
		void setup();
		void update();
		void draw();
        void onAnimQueueDone(ofxAnimatableQueueOfPoint::EventArg&) {
            motion.startPlaying(); //loop the animation endlessly
        }
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
    ofSetFrameRate(fps);
    ofBackground(ofColor::black);
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetVerticalSync(true);
    ofDisableArbTex();
    ofSetSmoothLighting(true);
    //ofDisableAlphaBlending();

    eyeAnimator.startPlaying();

    light.setup();
    light.setDiffuseColor(ofFloatColor(255.0, 0.0, 0.0f));
    light.setSpecularColor(ofColor(0, 0, 255));
    light.setDirectional();
    setLightOrientation(ofVec3f(0.0f, -80.0f, 00.0f));
    material.setShininess(120);
    material.setSpecularColor(ofColor::yellow);
    material.setEmissiveColor(ofColor::cornflowerBlue);
    material.setDiffuseColor(ofColor(255, 255, 255, 255));
    material.setAmbientColor(ofColor::white);

    // see size handler too
    windowResized(0,0);

    motion.add(ofPoint(0.0f, 0.0f), ofPoint(10.0f, 90.0f));
    motion.add(ofPoint(10.0f, 90.0f), ofPoint(-10.0f, 15.0f));
    motion.add(ofPoint(-10.0f, 15.0f), ofPoint(0.0f, 0.0f));
    motion.startPlaying(); //start the animation
}

//--------------------------------------------------------------
void ofApp::update(){
    motion.update(1.0f / fps);
    eyeAnimator.update(1.0f / fps);
    camera.setDistance(eyeAnimator.camera.getCurrentValue());
    light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
    // debug helper
    std::stringstream ss;
    ss << eyeAnimator.getIndex();
    ofSetWindowTitle(ss.str());
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    ofPushMatrix();
    ofPushStyle();
    light.enable();
    material.begin();
    camera.begin();
    eyeAnimator.bind();
    ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    ofRotateXDeg(180.0f); // hide seam
    motion.draw();
    ofDrawSphere(0.0f, 0.0f, 0.0f, r);
    eyeAnimator.unbind();
    camera.end();
    material.end();
    light.disable();
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







