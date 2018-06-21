<template>
  <div>
    <div id="divSideNav" class="sidenav">
      <a href="#">Page 1</a>
      <a href="#">Page 2</a>
    </div>
    <div id="main">
      <div class="row">
        <div class="topnav">
          <a class="active" href="#" v-on:click="toggleSidebar()">Menu(Esc)</a>
          <input type="text" placeholder="Search..." />
        </div>
      </div>
      <div class="row main-content">
        <p> Main Contents </p>
      </div>

    </div>
  </div>
</template>
<script>

export default {

  data: () => ({
    userid: null,
    menuVisible: false,
    selectedItem: null,
    items: [],
    selectedLayout: null,
  }),
  mounted: function() {    
    self = this;
    document.onkeydown = function(evt) {
      evt = evt || window.event;
      let isEsc = false;
      if ("key" in evt) {
        isEsc = evt.key === "Escape" || evt.key === "Esc";
      } else {
        isEsc = evt.keyCode == 27;
      }
      if (isEsc) {
        self.toggleSidebar();
      }
    };
  },
  methods: {
    toggleSidebar() {
      let width = document.getElementById("divSideNav").style.width;
      if (width === "0px") {
        this.openNav();
      } else this.closeNav();
    },
    openNav() {
      document.getElementById("divSideNav").style.width = "200px";
      document.getElementById("main").style.marginLeft = "200px";
    },
    closeNav() {
      document.getElementById("divSideNav").style.width = "0px";
      document.getElementById("main").style.marginTop = "0px";
      document.getElementById("main").style.marginLeft = "0px";
    }
  }
};
</script>



