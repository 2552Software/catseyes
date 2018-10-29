    
#pragma once

#include "ofMain.h"
#include "ofxAnimatableFloat.h"
#include "ofxAnimatableOfPoint.h"
#include "ofxAnimatableOfColor.h"
#include "ofxAnimatableQueue.h"
#include "ofxOpenCv.h"

const int imgWidth = 640;// 320; // the motion image from the camera
const int imgHeight = 480;//240;

// always knows it rotation coordindates
class SuperSphere : public ofSpherePrimitive {
public:
    void draw() {
        ofPushMatrix();
        rotate(currentRotation);
        ofSpherePrimitive::draw();
        ofPopMatrix();
    }
    void rotateTo(ofVec3f target) {
        ofLogNotice() << "rotateTo " << target;
       currentRotation = target;
    }
private:
    void rotate(ofVec3f target) { 
        ofRotateDeg(target.x, 1.0f, 0.0f, 0.0f);
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
            sphere.rotateTo(ofVec3f(0, calc(imgWidth-target.x, -30.0f, 30.0f, imgHeight)));
        }
        void update() {
            float f = 1.0f / fps;
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
                    if (blob.area > 400 && blob.area > max && blob.boundingRect.x > 1 && blob.boundingRect.y > 1) {  //x,y 1,1 is some sort of strange case
                        target = blob.centroid;
                        found = true;
                    }
                }
                if (found) {
                    setAngle(target);
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
            ofImage image;
            if (image.load(name)) {
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
    };


    class Light : public ofLight  {
    public:
        void setup() {
            ofLight::setup();
            setDiffuseColor(ofFloatColor(255.0, 0.0, 0.0f));
            setSpecularColor(ofColor(0, 0, 255));
            setDirectional();
            setOrientation(ofVec3f(0.0f, -80.0f, 00.0f));
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
            setSpecularColor(ofColor::yellow);
            setEmissiveColor(ofColor::blue);
            setDiffuseColor(ofColor::sandyBrown);
            setAmbientColor(ofColor::white);
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
        void setup() {
            ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
            ofSetBackgroundColor(ofColor::black);
            ofSetLogLevel(OF_LOG_NOTICE);
            ofLogToConsole();
            ofEnableLighting();
            ofEnableDepthTest();
            ofSetVerticalSync(true);
            ofDisableArbTex();
            ofSetSmoothLighting(true);
            ofDisableAlphaBlending();
            camera.setup();
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
            ss << camera.getDistance();
            ofSetWindowTitle(ss.str());
        }

        //--------------------------------------------------------------
        void draw() {
            ofPushStyle();

            light.enable();
            material.begin();
            camera.begin();
            ofPushMatrix();
            ofTranslate((ofGetWidth() / 2) - eyeAnimator.sphere.getRadius(), ofGetHeight() / 2 - eyeAnimator.sphere.getRadius()/2, 0);
            eyeAnimator.draw();
            ofPopMatrix();
            camera.end();
            eyeAnimator.contours.draw(ofGetScreenWidth(), ofGetScreenHeight());
            material.end();
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
