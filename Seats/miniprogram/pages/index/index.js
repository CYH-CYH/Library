Page({
  /**
   * 页面的初始数据
   */
  data: {
    /*是否可以登录*/
    isLogin:false,
    /*最长用户信息*/
    stuNummaxLen:17,
    /*用户头像图片*/
    headImgSrc:"https://gimg2.baidu.com/image_search/src=http%3A%2F%2Fc-ssl.duitang.com%2Fuploads%2Fitem%2F201906%2F29%2F20190629132138_tibjb.thumb.400_0.gif&refer=http%3A%2F%2Fc-ssl.duitang.com&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1613287425&t=c38f13ff0a10adabc0d4ad2ccb976279",
    /*首页背景图片*/
    bgImg:"https://gimg2.baidu.com/image_search/src=http%3A%2F%2F5b0988e595225.cdn.sohucs.com%2Fimages%2F20180912%2F6983759514ed4e7cbc740d90cd133b71.gif&refer=http%3A%2F%2F5b0988e595225.cdn.sohucs.com&app=2002&size=f9999,10000&q=a80&n=0&g=0n&fmt=jpeg?sec=1613294441&t=6c84a4db3a8dd78c6857bd4eea9a2600"
  },
  /*点击登录按钮要处理的事件。
  1、将表单上面的信息传送到服务器上面进行处理
  2、如果成功，那么跳转到首页*/
loginEvent:function(res){
  wx.switchTab({
    "url":'/pages/seat/seat'
  })
},
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    
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