Page({
  data: {
  /*进入页面的值*/
  position:"A1",
  /*a(b)一共有多少行座位*/
  allSeatColum:18,
  /*装座位的高度*/
  seatsHeight:90,
  /*作为渲染座位标的数组，我记得从数据库下来就是一个数组*/
   Rlist:[[1,0,0,1],[0,1,1,-1]],
   Llsi:[],
   /*选择座位图片的路径*/
   /*没有人选择时的图片*/
   seatImgPath0:"/icon/seat0.jpg",
   /*有人选择该作为时的路径*/
   seatImgPath1:"/icon/seat1.jpg",
   /*默认当前座位信息是没人*/
   nowSeatPath:"/icon/seat0.jpg"
  },
  /*响应点击事件*/
  selecteSeat:function(res){
    if(res.curren==0)
    {
      if(res.currentTarget.seatState=0)
      {
        wx.showModal({
          title: '提示',
          content: '确认选择座位：'+'请在30分钟到达图书馆',
          success (res) {
            if (res.confirm) {
              //上传数据
              console.log('用户点击确定')
            } else if (res.cancel) {
              console.log('用户点击取消')
            }
          }
        })
        
        
      }
    }
  },
  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {
    // console.log(this.data.Rlist[1])
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