/* Pure geometry helpers. Small clients keep actions visible before optional graphs. */
typedef struct {int margin,gap,width,cols,buttonW,buttonH,buttonTop,metricCols,metricW,metricH,metricTop,chartTop,chartH,editorTop,editorH;} DASH_LAYOUT;
static DASH_LAYOUT dash_layout(int w,int h){
    DASH_LAYOUT d={0};d.margin=w>=400?12:6;d.gap=4;d.width=w-2*d.margin;if(d.width<1)d.width=1;
    d.cols=w>=860?4:2;d.buttonW=(d.width-(d.cols-1)*d.gap)/d.cols;if(d.buttonW<1)d.buttonW=1;
    d.buttonH=h>=500?31:26;d.buttonTop=52;
    d.metricTop=d.buttonTop+(8/d.cols)*(d.buttonH+d.gap)+5;
    d.metricCols=w>=660?3:2;d.metricW=(d.width-(d.metricCols-1)*d.gap)/d.metricCols;if(d.metricW<1)d.metricW=1;
    d.metricH=22;
    d.chartTop=d.metricTop+(6/d.metricCols)*d.metricH+25;
    d.chartH=h-d.chartTop>=230?104:0;
    d.editorTop=d.chartTop+d.chartH+(d.chartH?6:0);
    d.editorH=h-d.editorTop-d.margin;if(d.editorH<0)d.editorH=0;
    return d;
}
static int dash_bar_height(unsigned value,int height){
    if(value>100||height<=0)return 0;
    return (int)((unsigned)height/100*value+((unsigned)height%100)*value/100);
}
