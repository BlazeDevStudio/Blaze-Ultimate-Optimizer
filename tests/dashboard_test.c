#include <assert.h>
#include <stdio.h>
#include "../src/dashboard_layout.inc"
int main(void){
    for(int w=320;w<=1920;w+=20)for(int h=350;h<=1080;h+=10){
        DASH_LAYOUT d=dash_layout(w,h);
        assert(d.cols==2||d.cols==4);
        assert(d.buttonW*d.cols+d.gap*(d.cols-1)<=d.width);
        assert(d.metricW*d.metricCols+d.gap*(d.metricCols-1)<=d.width);
        assert(d.metricTop>=d.buttonTop+(8/d.cols)*(d.buttonH+d.gap));
        assert(d.chartTop>=d.metricTop+(6/d.metricCols)*d.metricH);
        assert(d.editorTop>=d.chartTop+d.chartH);
        assert(d.editorTop+d.editorH<=h-d.margin);
        assert(d.editorH>0);
        if(d.chartH)assert(d.editorH>=100);
    }
    assert(!dash_layout(480,350).chartH&&dash_layout(835,650).chartH);
    assert(dash_layout(900,600).cols==4);
    assert(dash_bar_height(0,50)==0&&dash_bar_height(100,50)==50);
    assert(dash_bar_height(50,53)==26&&dash_bar_height(101,53)==0&&dash_bar_height(50,-1)==0);
    puts("PASS: dashboard geometry matrix and bar scaling");return 0;
}
