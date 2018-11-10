 #pragma nce

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "#pragma nce

#include "ofMain.h"
#include "#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

#define DATAPATH "runtime"

class Light : public ofLight {
public:
    void setup() {
        ofLight::setup();
        setDirectional();
        setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
        setPosition(0, 0, 2000);

        specularcolor.setColor(ofColor::mediumVioletRed);
        specularcolor.setDuration(1.0f);
        specularcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        specularcolor.setCurve(LINEAR);
        specularcolor.animateTo(ofColor::orangeRed);

        ambientcolor.setColor(ofColor::blue);
        ambientcolor.setDuration(1.5f);
        ambientcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        ambientcolor.setCurve(LINEAR);
        ambientcolor.animateTo(ofColor::red);
    }
    void update() {
        specularcolor.update(1.0f / ofGetTargetFrameRate());
        ambientcolor.update(1.0f / ofGetTargetFrameRate());

        setAmbientColor(ambientcolor.getCurrentColor());
        setSpecularColor(specularcolor.getCurrentColor());
        //setAmbientColor(ofColor::mediumVioletRed);
        ///setSpecularColor(ofColor::saddleBrown);
        ////setDiffuseColor(ofColor::pink);
    }
    void setOrientation(ofVec3f rot) {
        ofVec3f xax(1, 0, 0);
        ofVec3f yax(0, 1, 0);
        ofVec3f zax(0, 0, 1);
        ofQuaternion q;
        q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
        ofLight::setOrientation(q);
    }
private:
    ofxAnimatableOfColor specularcolor;
    ofxAnimatableOfColor ambientcolor;

};

class Camera : public ofEasyCam {
public:
    void setup() {
        setDistance(4286); // magic number 
    }
};

class Material : public ofMaterial {
public:
    Material() {
        setup();
    }
    void setup() {
        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);// COLOR not used yet

        setShininess(120);
        setSpecularColor(ofColor::white);
        // setEmissiveColor(ofColor::black);
        setDiffuseColor(ofColor::whiteSmoke);
        setAmbientColor(ofColor::navajoWhite);
    }
    void update(){
        color.update(1.0f / ofGetTargetFrameRate());
    }
private:
    ofxAnimatableOfColor color; // not used yet

};

class Eye : public ofTexture {
public:
    Eye() {}
    Eye(const string&name) {
        setup(name);
    }
    Material material;

    void setup(const string&texName) {

        if (ofLoadImage(*this, texName)) {
            ofLogNotice("Eye") << "loaded " << texName;
        }
        else {
            ofLogError("Eye") << "not loaded " << texName;
        }
        //assimp not supported model.loadModel(objName);
    }
    void update() {
    }
    void start() {
        //color.applyCurrentColor();
        material.begin();
        bind();
    }
    void stop() {
        unbind();
        material.end();
    }
};
// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    SuperSphere() {}
    void setup(const string&name, const string&blinkPath) {
        eye.setup(name);
        string path = DATAPATH;
        path += "\\"+ blinkPath +".blink";
        ofDirectory dir(path);
        dir.listDir();
        blink.push_back(Eye(name));
        std::string name2;
        for (size_t i = 0; i < dir.size(); i++) {
            name2 = path + "\\";
            name2 += dir.getName(i);
            blink.push_back(Eye(name2));
        }
        blink.push_back(Eye(name2)); // last one gets skipped

        blinker.reset(0.0f);
        blinker.setCurve(LINEAR);
        blinker.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
        blinker.setDuration(0.2f);
        blinker.animateTo(blink.size() - 1);
       // blink
        setResolution(21);
        panDeg(180);
       // animateToAfterDelay
    }
    void update() {
        blinker.update(1.0f / ofGetTargetFrameRate());
        if (!blinker.isOrWillBeAnimating()) {
            blinker.reset(0.0f);
            blinker.animateToAfterDelay(blink.size() - 1, ofRandom(100));
        }
    }
    void scale() {
        //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    }
    void draw() {
        scale();
        std::stringstream ss;
        if (blinker.getCurrentValue()  > 0) {
            ss << blinker.getCurrentValue();
            ofSetWindowTitle(ss.str());
        }

       
        //if (blinker.isAnimating()) {
            blink[blinker.getCurrentValue()].start();
       // }
       // else {
            //eye.start();
       // }
        rotate();
        ofSpherePrimitive::draw();
       // if (blinker.isAnimating()) {
            blink[blinker.getCurrentValue()].stop();
       // }
      //  else {
           // eye.stop();
     //   }
    }
    void set(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
    void rotate() {
        rotate(currentRotation);
    }
private:
    void rotate(ofVec3f target) {
        std::stringstream ss;
        //ss << target;
        //ofSetWindowTitle(ss.str());
       // if (fabs(target.x) > 16.0)
            ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        //if (fabs(target.y) > 16.0)
            ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
    Eye eye; 
    std::vector<Eye> blink;
    ofxAnimatableFloat blinker; // blink animation
};

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 2) {
            ofLogNotice() << " cap list size to 3 ";
            animSteps.pop_back(); // only keep the  most recent
        }
        animSteps.push_back(targetValue);//bugbug go to pointer
    }
    ofxAnimatableOfPoint getCurrentValue() {
        return anim;
    }

    void update() {
        for (ofxAnimatableOfPoint &step : animSteps) {
            step.update(1.0f / ofGetTargetFrameRate());
        }
        anim.update(1.0f / ofGetTargetFrameRate());

        if (playing) {
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
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.25);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
        startPlaying();
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
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
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
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
    private:
        ofVideoGrabber video;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    };

    class ImageAnimator {
    public:
        void setup() {
            ofSetFrameRate(60.0f);
            
           // animator.startAutoUpdate();
            animator.reset(0.0f);
            animator.setDuration(1.0f);
            animator.setRepeatType(LOOP_BACK_AND_FORTH);
            animator.setCurve(LINEAR);

          //  camera.startAutoUpdate();
            camera.reset(500.0f);
            camera.setDuration(5.0f);
            camera.setRepeatType(LOOP_BACK_AND_FORTH);
            camera.setCurve(EASE_IN);

            contours.setup();
            startPlaying();

        }
        // convert point on X to a rotation
        float turnToX(float x) {
            return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
        }
        float turnToY(float y) {
            return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
        }
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void setAngle(ofVec3f target) {
            //path.append(ofVec3f(calc(target.y, imgWidth), calc(target.x, imgHeight)));
            //sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(target.x, -40.0f, 45.0f, imgHeight)));
            for (SuperSphere&eye : eyes) {
                eye.set(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(imgWidth - target.x, -20.0f, 20.0f, imgHeight)));
            }
        }
        void update() {
            for (SuperSphere&eye : eyes) {
                eye.update();
            }
            float max = 0.0f;
            contours.update();
            if (contours.contourFinder.blobs.size() > 0) {
                ofDefaultVec3 target;
                bool found = false;
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > 100 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
                }
            }
        }
        void windowResized(int w, int h) {
            for (SuperSphere&eye : eyes) {
                eye.setRadius(std::min(w, h));
            }
        }
        void draw() {
            getCurrentEyeRef().draw();
        }
        void startPlaying() {
            string path = DATAPATH;
            ofDirectory dir(path);
            dir.allowExt("png");
            dir.allowExt("jpg");
            dir.listDir();
            size_t i = 0;
            for (; i < dir.size(); i++) {
                add(dir.getPath(i), dir.getName(i));
            }
            animator.animateTo(eyes.size()-1);
        }
        SuperSphere& getCurrentEyeRef() {
            return eyes[(int)animator.getCurrentValue()];
        }
        void add(const std::string &name, const std::string &root) {
                std::string::size_type pos = root.find('.');
                std::string blinkPath;
                if (pos != std::string::npos) {
                    blinkPath = root.substr(0, pos);
                }
                else {
                    blinkPath = root;
                }
            eyes.push_back(SuperSphere());
            eyes[eyes.size()-1].setup(name, blinkPath);
        }
        ContoursBuilder contours;
        ofxAnimatableFloat camera;
        ofxAnimatableFloat animator;
        ofxAnimatableQueueOfPoint path; // path of image

    private:
        std::vector<SuperSphere> eyes;
    };



class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        ofMesh m;
        void setup() {
            //ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::black);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            //ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            //ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
           // camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            light.enable();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.getCurrentEyeRef().getRadius(), ofGetHeight() / 2 - eyeAnimator.getCurrentEyeRef().getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};
.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

#define DATAPATH "runtime"

class Light : public ofLight {
public:
    void setup() {
        ofLight::setup();
        setDirectional();
        setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
        setPosition(0, 0, 2000);

        specularcolor.setColor(ofColor::mediumVioletRed);
        specularcolor.setDuration(1.0f);
        specularcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        specularcolor.setCurve(LINEAR);
        specularcolor.animateTo(ofColor::orangeRed);

        ambientcolor.setColor(ofColor::blue);
        ambientcolor.setDuration(1.5f);
        ambientcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        ambientcolor.setCurve(LINEAR);
        ambientcolor.animateTo(ofColor::red);
    }
    void update() {
        specularcolor.update(1.0f / ofGetTargetFrameRate());
        ambientcolor.update(1.0f / ofGetTargetFrameRate());

        setAmbientColor(ambientcolor.getCurrentColor());
        setSpecularColor(specularcolor.getCurrentColor());
        //setAmbientColor(ofColor::mediumVioletRed);
        ///setSpecularColor(ofColor::saddleBrown);
        ////setDiffuseColor(ofColor::pink);
    }
    void setOrientation(ofVec3f rot) {
        ofVec3f xax(1, 0, 0);
        ofVec3f yax(0, 1, 0);
        ofVec3f zax(0, 0, 1);
        ofQuaternion q;
        q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
        ofLight::setOrientation(q);
    }
private:
    ofxAnimatableOfColor specularcolor;
    ofxAnimatableOfColor ambientcolor;

};

class Camera : public ofEasyCam {
public:
    void setup() {
        setDistance(4286); // magic number 
    }
};

class Material : public ofMaterial {
public:
    Material() {
        setup();
    }
    void setup() {
        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);// COLOR not used yet

        setShininess(120);
        setSpecularColor(ofColor::white);
        // setEmissiveColor(ofColor::black);
        setDiffuseColor(ofColor::whiteSmoke);
        setAmbientColor(ofColor::navajoWhite);
    }
    void update(){
        color.update(1.0f / ofGetTargetFrameRate());
    }
private:
    ofxAnimatableOfColor color; // not used yet

};

class Eye : public ofTexture {
public:
    Eye() {}
    Eye(const string&name) {
        setup(name);
    }
    Material material;

    void setup(const string&texName) {

        if (ofLoadImage(*this, texName)) {
            ofLogNotice("Eye") << "loaded " << texName;
        }
        else {
            ofLogError("Eye") << "not loaded " << texName;
        }
        //assimp not supported model.loadModel(objName);
    }
    void update() {
    }
    void start() {
        //color.applyCurrentColor();
        material.begin();
        bind();
    }
    void stop() {
        unbind();
        material.end();
    }
};
// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    SuperSphere() {}
    void setup(const string&name, const string&blinkPath) {
        eye.setup(name);
        string path = DATAPATH;
        path += "\\"+ blinkPath +".blink";
        ofDirectory dir(path);
        dir.listDir();
        blink.push_back(Eye(name));
        for (size_t i = 0; i < dir.size(); i++) {
            std::string name2 = path + "\\";
            name2 += dir.getName(i);
            blink.push_back(Eye(name2));
        }

        blinker.reset(0.0f);
        blinker.setCurve(LINEAR);
        blinker.setRepeatType(LOOP_BACK_AND_FORTH_ONCE);
        blinker.setDuration(1.0f);
        blinker.animateTo(blink.size() - 1);
       // blink
        setResolution(21);
        panDeg(180);
       // animateToAfterDelay
    }
    void update() {
        blinker.update(1.0f / ofGetTargetFrameRate());
        if (!blinker.isOrWillBeAnimating()) {
            blinker.reset(0.0f);
            blinker.animateTo(blink.size() - 1);
        }
    }
    void scale() {
        //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    }
    void draw() {
        scale();
        std::stringstream ss;
        if (blinker.getCurrentValue()  > 0) {
            ss << blinker.getCurrentValue();
            ofSetWindowTitle(ss.str());
        }

       
        //if (blinker.isAnimating()) {
            blink[blinker.getCurrentValue()].start();
       // }
       // else {
            //eye.start();
       // }
        rotate();
        ofSpherePrimitive::draw();
       // if (blinker.isAnimating()) {
            blink[blinker.getCurrentValue()].stop();
       // }
      //  else {
           // eye.stop();
     //   }
    }
    void set(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
    void rotate() {
        rotate(currentRotation);
    }
private:
    void rotate(ofVec3f target) {
        std::stringstream ss;
        //ss << target;
        //ofSetWindowTitle(ss.str());
       // if (fabs(target.x) > 16.0)
            ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        //if (fabs(target.y) > 16.0)
            ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
    Eye eye; 
    std::vector<Eye> blink;
    ofxAnimatableFloat blinker; // blink animation
};

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 2) {
            ofLogNotice() << " cap list size to 3 ";
            animSteps.pop_back(); // only keep the  most recent
        }
        animSteps.push_back(targetValue);//bugbug go to pointer
    }
    ofxAnimatableOfPoint getCurrentValue() {
        return anim;
    }

    void update() {
        for (ofxAnimatableOfPoint &step : animSteps) {
            step.update(1.0f / ofGetTargetFrameRate());
        }
        anim.update(1.0f / ofGetTargetFrameRate());

        if (playing) {
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
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.25);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
        startPlaying();
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
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
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
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
    private:
        ofVideoGrabber video;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    };

    class ImageAnimator {
    public:
        void setup() {
            ofSetFrameRate(60.0f);
            
           // animator.startAutoUpdate();
            animator.reset(0.0f);
            animator.setDuration(1.0f);
            animator.setRepeatType(LOOP_BACK_AND_FORTH);
            animator.setCurve(LINEAR);

          //  camera.startAutoUpdate();
            camera.reset(500.0f);
            camera.setDuration(5.0f);
            camera.setRepeatType(LOOP_BACK_AND_FORTH);
            camera.setCurve(EASE_IN);

            contours.setup();
            startPlaying();

        }
        // convert point on X to a rotation
        float turnToX(float x) {
            return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
        }
        float turnToY(float y) {
            return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
        }
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void setAngle(ofVec3f target) {
            //path.append(ofVec3f(calc(target.y, imgWidth), calc(target.x, imgHeight)));
            //sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(target.x, -40.0f, 45.0f, imgHeight)));
            for (SuperSphere&eye : eyes) {
                eye.set(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(imgWidth - target.x, -20.0f, 20.0f, imgHeight)));
            }
        }
        void update() {
            for (SuperSphere&eye : eyes) {
                eye.update();
            }
            float max = 0.0f;
            contours.update();
            if (contours.contourFinder.blobs.size() > 0) {
                ofDefaultVec3 target;
                bool found = false;
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > 100 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
                }
            }
        }
        void windowResized(int w, int h) {
            for (SuperSphere&eye : eyes) {
                eye.setRadius(std::min(w, h));
            }
        }
        void draw() {
            getCurrentEyeRef().draw();
        }
        void startPlaying() {
            string path = DATAPATH;
            ofDirectory dir(path);
            dir.allowExt("png");
            dir.allowExt("jpg");
            dir.listDir();
            size_t i = 0;
            for (; i < dir.size(); i++) {
                add(dir.getPath(i), dir.getName(i));
            }
            animator.animateTo(eyes.size()-1);
        }
        SuperSphere& getCurrentEyeRef() {
            return eyes[(int)animator.getCurrentValue()];
        }
        void add(const std::string &name, const std::string &root) {
                std::string::size_type pos = root.find('.');
                std::string blinkPath;
                if (pos != std::string::npos) {
                    blinkPath = root.substr(0, pos);
                }
                else {
                    blinkPath = root;
                }
            eyes.push_back(SuperSphere());
            eyes[eyes.size()-1].setup(name, blinkPath);
        }
        ContoursBuilder contours;
        ofxAnimatableFloat camera;
        ofxAnimatableFloat animator;
        ofxAnimatableQueueOfPoint path; // path of image

    private:
        std::vector<SuperSphere> eyes;
    };



class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        ofMesh m;
        void setup() {
            //ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::black);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            //ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            //ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
           // camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            light.enable();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.getCurrentEyeRef().getRadius(), ofGetHeight() / 2 - eyeAnimator.getCurrentEyeRef().getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};
.h"
#include "ofxOpenCv.h"#pragma nce

#include "ofMain.h"
#include "#pragma nce

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;
const float fps = 60.0f;

class Light : public ofLight {
public:
    void setup() {
        ofLight::setup();
        setDirectional();
        setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
        setPosition(0, 0, 2000);

        specularcolor.setColor(ofColor::mediumVioletRed);
        specularcolor.setDuration(1.0f);
        specularcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        specularcolor.setCurve(LINEAR);
        specularcolor.animateTo(ofColor::orangeRed);

        ambientcolor.setColor(ofColor::blue);
        ambientcolor.setDuration(1.5f);
        ambientcolor.setRepeatType(LOOP_BACK_AND_FORTH);
        ambientcolor.setCurve(LINEAR);
        ambientcolor.animateTo(ofColor::red);
    }
    void update() {
        specularcolor.update(1.0f / fps);
        setAmbientColor(ambientcolor.getCurrentColor());
        setSpecularColor(specularcolor.getCurrentColor());
        //setAmbientColor(ofColor::mediumVioletRed);
        ///setSpecularColor(ofColor::saddleBrown);
        ////setDiffuseColor(ofColor::pink);
    }
    void setOrientation(ofVec3f rot) {
        ofVec3f xax(1, 0, 0);
        ofVec3f yax(0, 1, 0);
        ofVec3f zax(0, 0, 1);
        ofQuaternion q;
        q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
        ofLight::setOrientation(q);
    }
private:
    ofxAnimatableOfColor specularcolor;
    ofxAnimatableOfColor ambientcolor;

};

class Camera : public ofEasyCam {
public:
    void setup() {
        setDistance(4286); // magic number 
    }
};

class Material : public ofMaterial {
public:
    Material() {
        setup();
    }
    void setup() {
        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);

        setShininess(120);
        setSpecularColor(ofColor::white);
        // setEmissiveColor(ofColor::black);
        setDiffuseColor(ofColor::whiteSmoke);
        setAmbientColor(ofColor::navajoWhite);
    }
    void update(){
    }
private:
    ofxAnimatableOfColor color; // not used yet

};

class Eye : public ofTexture {
public:
    Eye() {}
    Eye(const string&name) {
        setup(name);
    }
    Material material;

    void setup(const string&texName) {

        if (ofLoadImage(*this, texName)) {
            ofLogNotice("Eye") << "loaded " << texName;
            generateMipmap();
            setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        }
        else {
            ofLogError("Eye") << "not loaded " << texName;
        }
        //assimp not supported model.loadModel(objName);
    }
    void update() {
    }
    void start() {
        //color.applyCurrentColor();
        material.begin();
        bind();
    }
    void stop() {
        unbind();
        material.end();
    }
};
// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    SuperSphere() {}
    SuperSphere(const string&name) {
        setup(name);
    }
    void setup(const string&name) {
        eye.setup(name);
        setResolution(21);
        panDeg(180);
    }
    void update() {
    }
    void scale() {
        //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    }
    void draw() {
        scale();
        eye.start();
        rotate();
        ofSpherePrimitive::draw();
        eye.stop();
    }
    void set(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
    void rotate() {
        rotate(currentRotation);
    }
private:
    void rotate(ofVec3f target) {
        std::stringstream ss;
        ss << target;
        ofSetWindowTitle(ss.str());
       // if (fabs(target.x) > 16.0)
            ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        //if (fabs(target.y) > 16.0)
            ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
    Eye eye; //"iris\\tom.jpg"
};

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 2) {
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
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.25);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
        startPlaying();
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
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
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
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
    private:
        ofVideoGrabber video;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    };

    class ImageAnimator {
    public:
        void setup() {
            ofSetFrameRate(fps);
            
            animator.reset(0.0f);
            animator.setDuration(1.0f);
            animator.setRepeatType(LOOP);
            animator.setCurve(LINEAR);

            camera.reset(500.0f);
            camera.setDuration(5.0f);
            camera.setRepeatType(LOOP_BACK_AND_FORTH);
            camera.setCurve(EASE_IN);

            contours.setup();
            startPlaying();

        }
        // convert point on X to a rotation
        float turnToX(float x) {
            return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
        }
        float turnToY(float y) {
            return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
        }
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void setAngle(ofVec3f target) {
            //path.append(ofVec3f(calc(target.y, imgWidth), calc(target.x, imgHeight)));
            //sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(target.x, -40.0f, 45.0f, imgHeight)));
            for (SuperSphere&eye : eyes) {
                eye.set(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(imgWidth - target.x, -20.0f, 20.0f, imgHeight)));
            }
        }
        void update() {
            float f = 1.0f / fps;
            for (SuperSphere&eye : eyes) {
                eye.update();
            }
            animator.update(f);
            camera.update(f);
            path.update(f);
            float max = 0.0f;
            contours.update();
            if (contours.contourFinder.blobs.size() > 0) {
                ofDefaultVec3 target;
                bool found = false;
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > 100 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
                }
            }
        }
        void windowResized(int w, int h) {
            for (SuperSphere&eye : eyes) {
                eye.setRadius(std::min(w, h));
            }
        }
        void draw() {
            getCurrentEyeRef().draw();
        }
        void startPlaying() {
            string path = "runtime";
            ofDirectory dir(path);
            dir.allowExt("png");
            dir.allowExt("jpg");
            dir.listDir();
            for (size_t i = 0; i < dir.size(); i++) {
                add(dir.getPath(i));
            }

            animator.animateTo(eyes.size());
        }
        SuperSphere& getCurrentEyeRef() {
            return eyes[(int)animator.getCurrentValue()];
        }
        void add(const std::string &name) {
            eyes.push_back(SuperSphere(name));
        }
        ContoursBuilder contours;
        ofxAnimatableFloat camera;
        ofxAnimatableFloat animator;
        ofxAnimatableQueueOfPoint path; // path of image

    private:
        std::vector<SuperSphere> eyes;
    };



class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        ofMesh m;
        void setup() {
            //ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::blue);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            //ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            //ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
           // camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            light.enable();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.getCurrentEyeRef().getRadius(), ofGetHeight() / 2 - eyeAnimator.getCurrentEyeRef().getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};
.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;
const float fps = 60.0f;

class Material : public ofMaterial {
public:
    void setup() {
        setShininess(120);
        setSpecularColor(ofColor::white);
        // setEmissiveColor(ofColor::black);
        setDiffuseColor(ofColor::whiteSmoke);
        setAmbientColor(ofColor::navajoWhite);
    }
};

class Eye : public ofTexture {
public:
    Eye() {}
    Eye(const string&name) {
        setup(name);
    }
    Material material;
    ofxAnimatableOfColor color; // image  colors

    void setup(const string&texName) {
        color.setColor(ofColor::white);
        color.setDuration(1.0f);
        color.setRepeatType(LOOP_BACK_AND_FORTH);
        color.setCurve(LINEAR);
        color.animateTo(ofColor::orangeRed);

        material.setup();

        if (ofLoadImage(*this, texName)) {
            ofLogNotice("Eye") << "loaded " << texName;
            generateMipmap();
            setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        }
        else {
            ofLogError("Eye") << "not loaded " << texName;
        }
        //assimp not supported model.loadModel(objName);
    }
    void update() {
        color.update(1.0f/fps);
    }
    void start() {
        ofSetColor(255);
        //color.applyCurrentColor();
        material.begin();
        bind();
    }
    void stop() {
        unbind();
        material.end();
    }
};
// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    SuperSphere() {}
    SuperSphere(const string&name) {
        setup(name);
    }
    void setup(const string&name) {
        eye.setup(name);
        setResolution(21);
        panDeg(180);
    }
    void update() {
    }
    void scale() {
        //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
    }
    void draw() {
        scale();
        eye.start();
        rotate();
        ofSpherePrimitive::draw();
        eye.stop();
    }
    void set(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
    void rotate() {
        rotate(currentRotation);
    }
private:
    void rotate(ofVec3f target) {
        std::stringstream ss;
        ss << target;
        ofSetWindowTitle(ss.str());
       // if (fabs(target.x) > 16.0)
            ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        //if (fabs(target.y) > 16.0)
            ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
    Eye eye; //"iris\\tom.jpg"
};

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 2) {
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
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.25);
        targetPoint.setRepeatType(PLAY_ONCE);
        targetPoint.setCurve(LINEAR);
        addTransition(targetPoint);
        startPlaying();
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
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
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
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
    private:
        ofVideoGrabber video;
        ofxCvColorImage colorImg;
        ofxCvGrayscaleImage grayImage, backgroundImage, grayDiff;
    };

    class ImageAnimator {
    public:
        void setup() {
            ofSetFrameRate(fps);
            
            animator.reset(0.0f);
            animator.setDuration(1.0f);
            animator.setRepeatType(LOOP);
            animator.setCurve(LINEAR);

            camera.reset(500.0f);
            camera.setDuration(5.0f);
            camera.setRepeatType(LOOP_BACK_AND_FORTH);
            camera.setCurve(EASE_IN);

            contours.setup();
            startPlaying();

        }
        // convert point on X to a rotation
        float turnToX(float x) {
            return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
        }
        float turnToY(float y) {
            return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
        }
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void setAngle(ofVec3f target) {
            //path.append(ofVec3f(calc(target.y, imgWidth), calc(target.x, imgHeight)));
            //sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(target.x, -40.0f, 45.0f, imgHeight)));
            for (SuperSphere&eye : eyes) {
                eye.set(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(imgWidth - target.x, -20.0f, 20.0f, imgHeight)));
            }
        }
        void update() {
            float f = 1.0f / fps;
            for (SuperSphere&eye : eyes) {
                eye.update();
            }
            animator.update(f);
            camera.update(f);
            path.update(f);
            float max = 0.0f;
            contours.update();
            if (contours.contourFinder.blobs.size() > 0) {
                ofDefaultVec3 target;
                bool found = false;
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > 100 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
                }
            }
        }
        void windowResized(int w, int h) {
            for (SuperSphere&eye : eyes) {
                eye.setRadius(std::min(w, h));
            }
        }
        void draw() {
            getCurrentEyeRef().draw();
        }
        void startPlaying() {
            string path = "runtime";
            ofDirectory dir(path);
            dir.allowExt("png");
            dir.allowExt("jpg");
            dir.listDir();
            for (size_t i = 0; i < dir.size(); i++) {
                add(dir.getPath(i));
            }

            animator.animateTo(eyes.size());
        }
        SuperSphere& getCurrentEyeRef() {
            return eyes[(int)animator.getCurrentValue()];
        }
        void add(const std::string &name) {
            eyes.push_back(SuperSphere(name));
        }
        ContoursBuilder contours;
        ofxAnimatableFloat camera;
        ofxAnimatableFloat animator;
        ofxAnimatableQueueOfPoint path; // path of image

    private:
        std::vector<SuperSphere> eyes;
    };


    class Light : public ofLight  {
    public:
        void setup() {
            ofLight::setup();
            setAreaLight(2000, 2000);
            ////setDiffuseColor(ofColor::pink);
            setAmbientColor(ofColor::mediumVioletRed);
            setSpecularColor(ofColor::saddleBrown);
           //setDirectional();
           setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
           setPosition(0, 0, 2000);
        }
        void setOrientation(ofVec3f rot) {
            ofVec3f xax(1, 0, 0);
            ofVec3f yax(0, 1, 0);
            ofVec3f zax(0, 0, 1);
            ofQuaternion q;
            q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
            ofLight::setOrientation(q);
        }
    };

    class Camera : public ofEasyCam {
    public:
        void setup() {
            setDistance(4286); // magic number 
        }
    };

class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        ofMesh m;
        void setup() {
            //ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::blue);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            //ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            //ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
           // camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            //light.enable();
           // material.begin();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.getCurrentEyeRef().getRadius(), ofGetHeight() / 2 - eyeAnimator.getCurrentEyeRef().getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
           // material.end();
           // light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};

#include "ofxAssimpModelLoader.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

class myEye : public ofxAssimpModelLoader {
public:
    void draw(ofPolyRenderMode renderType) {
        if (scene == NULL) {
            return;
        }

        ofPushStyle();

        ofPushMatrix();
        ofMultMatrix(modelMatrix);

#ifndef TARGET_OPENGLES
        glPolygonMode(GL_FRONT_AND_BACK, ofGetGLPolyMode(renderType));
#endif
        ofRotateDeg(180.0f, 0.0f, 1.0f, 0.0f);
        for (size_t i = 0; i < modelMeshes.size(); i++) {
            ofxAssimpMeshHelper & mesh = modelMeshes[i];
            // item 2 needs to move 2*r in the Z
            if (i != 1) { //1 may be good enough (2 is pupil)
                ofRotateDeg(180.0f, 0.0f, 1.0f, 0.0f);
                // ofTranslate(0.0, 0.0, 100);
                // item 2 needs to move 2*r in the Z
            }

            ofPushMatrix();
            ofMultMatrix(mesh.matrix);

            if (bUsingTextures) {
                if (mesh.hasTexture()) {
                    mesh.getTextureRef().bind();
                }
            }

            if (bUsingMaterials) 
                mesh.material.begin();
            }

            if (mesh.twoSided) {
                glEnable(GL_CULL_FACE);
            }
            else {
                glDisable(GL_CULL_FACE);
            }

            ofEnableBlendMode(mesh.blendMode);

#ifndef TARGET_OPENGLES
            mesh.vbo.drawElements(GL_TRIANGLES, mesh.indices.size());
#else
            switch (renderType) {
            case OF_MESH_FILL:
                mesh.vbo.drawElements(GL_TRIANGLES, mesh.indices.size());
                break;
            case OF_MESH_WIREFRAME:
                //note this won't look the same as on non ES renderers.
                //there is no easy way to convert GL_TRIANGLES to outlines for each triangle
                mesh.vbo.drawElements(GL_LINES, mesh.indices.size());
                break;
            case OF_MESH_POINTS:
                mesh.vbo.drawElements(GL_POINTS, mesh.indices.size());
                break;
        }
#endif

            if (bUsingTextures) {
                if (mesh.hasTexture()) {
                    mesh.getTextureRef().unbind();
                }
            }

            if (bUsingMaterials) {
                mesh.material.end();
            }

            ofPopMatrix();
    }

#ifndef TARGET_OPENGLES
        //set the drawing mode back to FILL if its drawn the model with a different mode.
        if (renderType != OF_MESH_FILL) {
            glPolygonMode(GL_FRONT_AND_BACK, ofGetGLPolyMode(OF_MESH_FILL));
        }
#endif

        ofPopMatrix();
        ofPopStyle();
}

};

// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    void setup() {
        //rotateDeg(180.0f, 0.0f, 1.0f, 0.0f); // flip to front to hide seam
        //setResolution(25);
        //setRadius(std::min(ofGetWidth(), ofGetHeight()));

        model.loadModel("eyeball.obj");//
    }
    void update() {
        model.update();
    }
    void draw() {
        ofSetColor(255);

        ofEnableBlendMode(OF_BLENDMODE_ALPHA);

        ofEnableDepthTest();
#ifndef TARGET_PROGRAMMABLE_GL
        glShadeModel(GL_SMOOTH); //some model / light stuff
#endif

        model.draw(OF_MESH_FILL);
#ifndef TARGET_PROGRAMMABLE_GL
        glEnable(GL_NORMALIZE);
#endif
        ofPushMatrix();

        ofxAssimpMeshHelper & meshHelper = model.getMeshHelper(0);

        ofMultMatrix(model.getModelMatrix());
        ofMultMatrix(meshHelper.matrix);

        ofMaterial & material = meshHelper.material;
        if (meshHelper.hasTexture()) {
            meshHelper.getTextureRef().bind();
        }
        material.begin();
        model.drawFaces();
        material.end();
        if (meshHelper.hasTexture()) {
            meshHelper.getTextureRef().unbind();
        }
        ofPopMatrix();

        return;

        rotate(currentRotation);
        //ofSpherePrimitive::draw();
        float r = getRadius();
        //ofTranslate(0, 0, 5000);
        ofPopStyle();
        ofPopMatrix();
    }
    void rotateTo(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
private:
    myEye model;
    void rotate(ofVec3f target) { 
        std::stringstream ss;
        ss << target;
        ofSetWindowTitle(ss.str());
       // if (fabs(target.x) > 16.0)
            ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
        //if (fabs(target.y) > 16.0)
            ofRotateDeg(target.y, 0.0f, 1.0f, 0.0f);
        ofRotateDeg(target.z, 0.0f, 0.0f, 1.0f);
    }
    ofVec3f currentRotation;
};

class ofxAnimatableQueueOfPoint {
public:
    ofxAnimatableQueueOfPoint() {
    }
    void addTransition(ofxAnimatableOfPoint targetValue) {
        ofLogNotice() << "addTransition " << targetValue.getCurrentPosition();
        if (targetValue.getCurrentPosition().x == 0) {
            int i = 0;
        }
        if (animSteps.size() > 2) {
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
    void append(const ofVec3f& target) {
        ofxAnimatableOfPoint targetPoint;
        if (animSteps.size() > 0) {
            targetPoint.setPosition(getPoint());
        }
        targetPoint.animateTo(target);
        targetPoint.setDuration(0.25);
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
        void update() {
            video.update();
            if (video.isFrameNew()) { // && (ofGetFrameNum() & 1) to slow things down
                                      // clear less often
                colorImg.setFromPixels(video.getPixels());
                grayImage = colorImg; // convert our color image to a grayscale image
                if (backgroundImage.bAllocated) {
                    grayDiff.absDiff(backgroundImage, grayImage);
                }
                backgroundImage = grayImage; // only track new items -- so eye moves when objects move
                grayDiff.threshold(50); // turn any pixels above 30 white, and below 100 black
                if (!contourFinder.findContours(grayDiff, 5, (imgWidth*imgHeight), 128, false, true)) {
                    contourFinder.blobs.clear(); // removes echo but does it make things draw too fast?
                }
                grayImage.blurGaussian(3);
                grayImage.threshold(50);
                if (!contourDrawer.findContours(grayImage, 5, (imgWidth*imgHeight), 128, true)) {
                    contourDrawer.blobs.clear(); 
                }
            }
        }
        void draw(float cxScreen, float cyScreen) {
            ofPushStyle();
            ofPushMatrix();
            ofNoFill();
            ofSetLineWidth(1);// ofRandom(1, 5));
            for (auto& blob : contourDrawer.blobs) {
                ofPolyline line;
                for (int i = 0; i < blob.nPts; i++) {
                    line.addVertex((imgWidth - blob.pts[i].x), blob.pts[i].y);
                }
                line.close();
                line.scale(cxScreen / imgWidth, cyScreen / imgHeight);
                line.draw();
            }
            ofSetLineWidth(5);// ofRandom(1, 5));
            if (contourFinder.blobs.size() > 0){
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
            }
            ofPopMatrix();
            ofPopStyle();
        }

        ofxCvContourFinder contourFinder;
        ofxCvContourFinder contourDrawer;
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
            animator.setDuration(1.0f);
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
            sphere.setup();

        }
        // convert point on X to a rotation
        float turnToX(float x) {
            return ofMap(x, 0.0f, ofGetHeight(), -45.0f, 45.0f);
        }
        float turnToY(float y) {
            return ofMap(y, 0.0f, ofGetWidth(), -45.0f, 45.0f);
        }
        float calc(float val, float start, float end, float len) {
            float result = start;
            float factor = 0.10f;
            float inc = fabsf(start - end)/10;
            for (; true; result += inc, factor += 0.10f) {
                if (val< len*factor) {
                    break;
                }
            }
            ofLogNotice() << "result " << result << " source " << val << " of " << len;
            return result;
        }
        void setAngle(ofVec3f target) {
            //path.append(ofVec3f(calc(target.y, imgWidth), calc(target.x, imgHeight)));
            //sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(target.x, -40.0f, 45.0f, imgHeight)));
             sphere.rotateTo(ofVec3f(calc(target.y, -25.0f, 45.0f, imgWidth), calc(imgWidth - target.x, -20.0f, 20.0f, imgHeight)));
        }
        void update() {
            
            float f = 1.0f / fps;
            sphere.update();
            animator.update(f);
            color.update(f);
            camera.update(f);
            path.update(f);
            float max = 0.0f;
            contours.update();
            if (contours.contourFinder.blobs.size() > 0) {
                ofDefaultVec3 target;
                bool found = false;
                for (auto& blob : contours.contourFinder.blobs) {
                    if (blob.area > 100 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
                }
            }
            //for (ofTexture& image : images) {
              //  image.update(); // keep updated
           // }
        }
        void scale() {
            //ofScale(1.0f, 1.0f, 1.0f); // a bit oblong i figure
        }
        void windowResized(int w, int h) {
            sphere.setRadius(std::min(w, h));
        }
        void draw() {
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
            ofTexture image;
            if (ofLoadImage(image, name)) {
                image.generateMipmap();
                image.setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

                images.push_back(image);
            }
        }
        void add(ofTexture image) {
            images.push_back(image);
        }
        ofTexture& getImage() {
            return images[getIndex()];
        }
        int getIndex() {
            return (int)animator.getCurrentValue();
        }
        // allow for various eyes
        void unbind() {
            getImage().unbind();
        }
        void bind() {
            //color.applyCurrentColor();
            getImage().bind();
        }
        ContoursBuilder contours;
        ofxAnimatableFloat camera;
        ofxAnimatableFloat animator;
        ofxAnimatableOfColor color; // image  colors
        ofxAnimatableQueueOfPoint path; // path of image
        SuperSphere sphere; //ofSpherePrimitive

    private:
        std::vector<ofTexture> images;
    };


    class Light : public ofLight  {
    public:
        void setup() {
            ofLight::setup();
            ////setDiffuseColor(ofColor::pink);
            //setAmbientColor(ofColor::mediumVioletRed);
           // setSpecularColor(ofColor::saddleBrown);
           //setDirectional();
            //setPosition(-200, 200, -2000);
          // setOrientation(ofVec3f(-200.0f, 300.0f, 00.0f));
          // setPosition(0, 0, 2000);
        }
        void setOrientation(ofVec3f rot) {
            ofVec3f xax(1, 0, 0);
            ofVec3f yax(0, 1, 0);
            ofVec3f zax(0, 0, 1);
            ofQuaternion q;
            q.makeRotate(rot.x, xax, rot.y, yax, rot.z, zax);
            ofLight::setOrientation(q);
        }
    };
    class Material : public ofMaterial {
    public:
        void setup() {
            setShininess(120);
            setSpecularColor(ofColor::white);
           // setEmissiveColor(ofColor::black);
            setDiffuseColor(ofColor::whiteSmoke);
            setAmbientColor(ofColor::navajoWhite);
        }
    };
    class Camera : public ofEasyCam {
    public:
        void setup() {
            setDistance(4286); // magic number 
        }
    };

class ofApp : public ofBaseApp{

	public:
        Light	light;
        Camera camera;
        ImageAnimator eyeAnimator;
        Material material;
        ofMesh m;
        void setup() {
            ofEnableSeparateSpecularLight();
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::black);
            ofSetColor(ofColor::white);
            ofSetLogLevel(OF_LOG_VERBOSE);
            ofLogToConsole();
            ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
            camera.lookAt(eyeAnimator.sphere);
            eyeAnimator.setup();
            light.setup();
            material.setup();
        }
        //--------------------------------------------------------------
        void update() {
            eyeAnimator.update();
            //too much only use when needed camera.setDistance(eyeAnimator.camera.getCurrentValue());
            // light.setPosition(0, 0, eyeAnimator.camera.getCurrentValue() - 1500);
            // debug helper
            std::stringstream ss;
            //ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }
        
        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();
            light.enable();
            //material.begin();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.sphere.getRadius(), ofGetHeight() / 2 - eyeAnimator.sphere.getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            //material.end();
            light.disable();
            ofPopStyle();
            ofPopMatrix();
        }
        //--------------------------------------------------------------
        void windowResized(int w, int h) {
            eyeAnimator.windowResized(w, h);
        }

private:
};
