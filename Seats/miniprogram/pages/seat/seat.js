// pages/seat/seat.js
Page({

  data: {
    /*swiper相关属性*/
    indicatorDots: true,
    vertical: false,
    autoplay: false,
    interval: 2000,
    duration: 500,
    /*图书馆导航*/
    floor:["一楼A区","一楼b区",
          "二楼A区","二楼B区",
          "三楼A区","三楼B区",
          "四楼A区","四楼B区",
          "五楼A区","五楼B区",
          "六楼A区","六楼B区"],
    /*swiper的高度*/
    swiperHeight:150,
    /*总容器的高度*/
    containerHeight:500,
    /*view-scoll的高度*/
    svHeight:null,
    /*定时器*/
    timeOut:null
  },
  /*初始化组件swiper和ciew-scoll的高度*/
  /*vsHeight = contianerHeight - swiperHeight*/
initHeight:function(){
  var quary = wx.createSelectorQuery();//创建节点查询
  quary.select('.seatContainer').boundingClientRect((rect)=>{
    this.setData({
      containerHeight:rect.height
    });
  }).exec();
},
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    //初始化页面高度
    this.initHeight();
    setTimeout(()=>{
      var term = this.data.containerHeight - this.data.swiperHeight; 
      this.setData({
        svHeight:term 
      });
      console.log(term);
    },100);
  },
  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {

  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide: function () {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage: function () {

  }
})