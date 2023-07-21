include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt_sub
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/mqtt_sub
	CATEGORY:=Base system
	TITLE:=mqtt_sub
	DEPENDS:=+libmosquitto-ssl +libuci +libcurl
endef

define Package/mqtt_sub/description
	MQTT Subscriber
endef

define Package/mqtt_sub/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_sub $(1)/usr/bin
	$(INSTALL_BIN) ./files/mqtt_sub.init $(1)/etc/init.d/mqtt_sub
endef

$(eval $(call BuildPackage,mqtt_sub))
