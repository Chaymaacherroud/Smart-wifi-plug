import websockets.*;

WebsocketClient wsc;
int now;
int oldvalue1=0, oldvalue2=0;

HScrollbar hs1, hs2;  // Two scrollbars  
Button bt1, bt2;

void setup(){
  size(640,360);
  noStroke();
  
  wsc= new WebsocketClient(this, "ws://192.168.10.100:81/");
  now=millis();
  
  hs1 = new HScrollbar(50, height/2-12 , width-100, 24, 16);
  hs2 = new HScrollbar(50, height/2+128, width-100, 24, 16);
  bt1 = new Button("ON CH1",50, 190, 50, 20);
  bt2 = new Button("ON CH2",50, 330, 50, 20);
    
}

void draw(){
  background(255);
  textSize(36);
  text("Dimmer Multi-Channel",100,50);
  textSize(22);
  text("Channel 1:",50,150);
  text("Channel 2:",50,290);
  textSize(12);
  
  hs1.update();
  hs2.update();
  hs1.display();
  hs2.display();
  bt1.bDraw();
  bt2.bDraw();
  
  stroke(0);
  
  float value1 = hs1.getPos();
  float value2 = hs2.getPos();
  int value_1 = (int)map(value1,53,591,0,100);
  int value_2 = (int)map(value2,53,591,0,100);
  text(value_1,312,140);
  text(value_2,312,280);
  
  if (oldvalue1 != value_1){
    oldvalue1 = value_1;
    String message_1 = "led"+str(1)+":"+str(value_1);
    println(message_1);
    wsc.sendMessage(message_1);
  }

  if (oldvalue2 != value_2){
    oldvalue2 = value_2;
    String message_2 = "led"+str(2)+":"+str(value_2);
    println(message_2);
    wsc.sendMessage(message_2);
  }


}

class HScrollbar {
  int swidth, sheight;    // width and height of bar
  float xpos, ypos;       // x and y position of bar
  float spos, newspos;    // x position of slider
  float sposMin, sposMax; // max and min values of slider
  int loose;              // how loose/heavy
  boolean over;           // is the mouse over the slider?
  boolean locked;
  float ratio;

  HScrollbar (float xp, float yp, int sw, int sh, int l) {
    swidth = sw;
    sheight = sh;
    int widthtoheight = sw - sh;
    ratio = (float)sw / (float)widthtoheight;
    xpos = xp;
    ypos = yp-sheight/2;
    spos = xpos;            // + swidth/2 - sheight/2;
    newspos = spos;
    sposMin = xpos;
    sposMax = xpos + swidth - sheight;
    loose = l;
  }
  void update() {
    if (overEvent()) {
      over = true;
    } else {
      over = false;
    }
    if (mousePressed && over) {
      locked = true;
    }
    if (!mousePressed) {
      locked = false;
    }
    if (locked) {
      newspos = constrain(mouseX-sheight/2, sposMin, sposMax);
    }
    if (abs(newspos - spos) > 1) {
      spos = spos + (newspos-spos)/loose;
    }
  }

  float constrain(float val, float minv, float maxv) {
    return min(max(val, minv), maxv);
  }

  boolean overEvent() {
    if (mouseX > xpos && mouseX < xpos+swidth &&
       mouseY > ypos && mouseY < ypos+sheight) {
      return true;
    } else {
      return false;
    }
  }

  void display() {
    noStroke();
    fill(204);
    rect(xpos, ypos, swidth, sheight);
    if (over || locked) {
      fill(0, 0, 0);
    } else {
      fill(102, 102, 102);
    }
    rect(spos, ypos, sheight, sheight);
  }

  float getPos() {
    // Convert spos to be values between
    // 0 and the total width of the scrollbar
    return spos * ratio;
  }
}

// the Button class
class Button {
  String label; // button label
  float x;      // top left corner x position
  float y;      // top left corner y position
  float w;      // width of button
  float h;      // height of button
  
  // constructor
  Button(String labelB, float xpos, float ypos, float widthB, float heightB) {
    label = labelB;
    x = xpos;
    y = ypos;
    w = widthB;
    h = heightB;
  }
  
  void bDraw() {
    fill(218);
    stroke(141);
    rect(x, y, w, h, 10);
    textAlign(CENTER, CENTER);
    fill(0);
    text(label, x + (w / 2), y + (h / 2));
    textAlign(LEFT);
  }
    
  boolean MouseIsOver() {
    if (mouseX > x && mouseX < (x + w) && mouseY > y && mouseY < (y + h)) {
      return true;
    }
    return false;
  }
}
