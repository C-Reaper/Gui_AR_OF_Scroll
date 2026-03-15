#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/RLCamera.h"
#include "/home/codeleaded/System/Static/Library/ImageFilter.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/OpticalFlow.h"

/*
Orientation:
4 Fingers pointing:
- up down
- right left
*/

#define OUTPUT_WIDTH    (RLCAMERA_WIDTH / 2)
#define OUTPUT_HEIGHT   (RLCAMERA_HEIGHT / 2)

RLCamera rlc;
OpticalFlow of;

Vec2 rect_p;
Vec2 rect_d;
Vec2 rect_v;
Vec2 rect_a;

void BW_Render(Pixel* target,unsigned int width,unsigned int height,float* bw_buffer,int w,int h){
    for(int i = 0;i<h;i++){
        for(int j = 0;j<w;j++){
            const float l = bw_buffer[i * w + j];
            target[i * width + j] = Pixel_toRGBA(l,l,l,1.0f);
        }
    }
}
void VF_Render(Pixel* target,unsigned int width,unsigned int height,Vec2* bw_buffer,int w,int h){
    for(int i = 0;i<h;i++){
        for(int j = 0;j<w;j++){
            const Vec2 flower = Vec2_Mulf(bw_buffer[i * w + j],0.5f);
            const Vec2 pos = { j,i };
            const float len = F32_Clamp(Vec2_Mag(flower),0.0f,1.0f);
            target[i * width + j] = Pixel_toRGBA(len,len,len,1.0f);
            const Vec2 tar = Vec2_Add(pos,Vec2_Mulf(Vec2_Norm(flower),5.0f));
            
            if(F32_Abs(flower.x) > F32_Abs(flower.y)){
                if(flower.x < 0.0f) RenderLine(pos,tar,GREEN,1.0f);
                else                RenderLine(pos,tar,BLUE,1.0f);
            }else{
                if(flower.y < 0.0f) RenderLine(pos,tar,RED,1.0f);
                else                RenderLine(pos,tar,YELLOW,1.0f);
            }
        }
    }
}

void Setup(AlxWindow* w){
    rlc = RLCamera_New("/dev/video0",RLCAMERA_WIDTH,RLCAMERA_HEIGHT);
    of = OpticalFlow_New(OUTPUT_WIDTH,OUTPUT_HEIGHT);
    
    rect_p = (Vec2){ OUTPUT_WIDTH * 0.5f,OUTPUT_HEIGHT * 0.5f };
    rect_d = (Vec2){ 10.0f,10.0f };
    rect_v = (Vec2){ 0.0f,0.0f };
    rect_a = (Vec2){ 0.0f,0.0f };
}
void Update(AlxWindow* w){
    if(RLCamera_Ready(&rlc)){
        RLCamera_Update(&rlc);
        OpticalFlow_Set(&of,&rlc.lastimg,w->ElapsedTime);
    }

    //const Vec2 max = OpticalFlow_Max(&of);
    //const Vec2 sig = Vec2_Mulf(OpticalFlow_Area(&of,max),0.5f);
    
    const Vec2 sig = OpticalFlow_Integrate(&of);
    const float l_sig = Vec2_Mag(sig);
    
    if(l_sig > 10.0f){
        rect_v = sig;
    }else{
        rect_v = (Vec2){ 0.0f,0.0f };
    }
    
    if(Stroke(ALX_KEY_W).DOWN)   rect_v.y =  1.0f;
    if(Stroke(ALX_KEY_S).DOWN)   rect_v.y = -1.0f;
    if(Stroke(ALX_KEY_A).DOWN)   rect_v.x =  1.0f;
    if(Stroke(ALX_KEY_D).DOWN)   rect_v.x = -1.0f;
    
    rect_v = Vec2_Add(rect_v,Vec2_Mulf(rect_a,1.0f * w->ElapsedTime));
    rect_p = Vec2_Add(rect_p,Vec2_Mulf(rect_v,10.0f * w->ElapsedTime));

    if(rect_p.x<0.0f){
        rect_p.x = 0.0f;
        rect_v.x *= -1.0f;
    }
    if(rect_p.y<0.0f){
        rect_p.y = 0.0f;
        rect_v.y *= -1.0f;
    }
    if(rect_p.x>OUTPUT_WIDTH - rect_d.x){
        rect_p.x = OUTPUT_WIDTH - rect_d.x;
        rect_v.x *= -1.0f;
    }
    if(rect_p.y>OUTPUT_HEIGHT - rect_d.y){
        rect_p.y = OUTPUT_HEIGHT - rect_d.y;
        rect_v.y *= -1.0f;
    }

    Clear(BLACK);

    VF_Render(WINDOW_STD_ARGS,of.flow,OUTPUT_WIDTH,OUTPUT_HEIGHT);
    //RenderLine(max,Vec2_Add(max,Vec2_Mulf(Vec2_Norm(of.flow[(int)max.y * of.captured.w + (int)max.x]),50.0f)),BLUE,1.0f);
    RenderRect(rect_p.x,rect_p.y,rect_d.x,rect_d.y,GREEN);

    printf("\r%f %f",sig.x,sig.y);
}
void Delete(AlxWindow* w){
    RLCamera_Free(&rlc);
    OpticalFlow_Free(&of);
}

int main(){
    if(Create("AR - Optical Flow - Scroll",OUTPUT_WIDTH,OUTPUT_HEIGHT,4,4,Setup,Update,Delete))
        Start();
    return 0;
}