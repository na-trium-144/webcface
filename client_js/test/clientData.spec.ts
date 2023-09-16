import { assert } from "chai";
import { ClientData, SyncDataStore2 } from "../src/clientData.js";

describe("ClientData Tests", function () {
  const selfName = "test";
  describe("SyncDataStore2", function () {
    let s2: SyncDataStore2<string>;
    beforeEach(function () {
      s2 = new SyncDataStore2<string>(selfName);
    });
    describe("#isSelf()", function () {
      it("returns true when self name is passed", function () {
        assert.isTrue(s2.isSelf(selfName));
      });
      it("returns false when different name is passed", function () {
        assert.isFalse(s2.isSelf("hoge"));
      });
      it("returns false when empty string is passed", function () {
        assert.isFalse(s2.isSelf(""));
      });
    });
    describe("#setSend()", function () {
      it("sets value to #dataSend", function () {
        s2.setSend("a", "b");
        assert.strictEqual(s2.dataSend.get("a"), "b");
      });
      it("sets value to #dataRecv", function () {
        s2.setSend("a", "b");
        assert.strictEqual(s2.dataRecv.get(selfName)?.get("a"), "b");
      });
    });
    describe("#setHidden()", function () {
      it("sets value to #dataSendHidden", function () {
        s2.setHidden("a", true);
        assert.isTrue(s2.dataSendHidden.get("a"));
      });
    });
    describe("#isHidden()", function () {
      it("returns false when it is set false", function () {
        s2.dataSendHidden.set("a", false);
        assert.isFalse(s2.isHidden("a"));
      });
      it("returns true when it is set true", function () {
        s2.dataSendHidden.set("a", true);
        assert.isTrue(s2.isHidden("a"));
      });
      it("returns false bt default", function () {
        assert.isFalse(s2.isHidden("a"));
      });
    });
    describe("#setRecv", function () {
      it("sets value to #dataRecv", function () {
        s2.setRecv("a", "b", "c");
        assert.strictEqual(s2.dataRecv.get("a")?.get("b"), "c");
      });
    });
    describe("#getRecv", function () {
      it("returns value in #dataRecv", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        assert.strictEqual(s2.getRecv("a", "b"), "c");
      });
      it("sets new request to #req and #reqSend", function () {
        s2.getRecv("a", "b");
        assert.strictEqual(s2.req.get("a")?.get("b"), 1);
        assert.strictEqual(s2.reqSend.get("a")?.get("b"), 1);
      });
      it("does not set #req and #reqSend if member is self name", function () {
        s2.getRecv(selfName, "b");
        assert.isEmpty(s2.req);
        assert.isEmpty(s2.reqSend);
      });
    });
    describe("#unsetRecv", function () {
      it("removes value from #dataRecv", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        s2.unsetRecv("a", "b");
        assert.isFalse(s2.dataRecv.get("a")?.has("b"));
      });
      it("sets 0 to #req and #reqSend if already set", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        s2.req.set("a", new Map([["b", 1]]));
        s2.reqSend.set("a", new Map([["b", 1]]));
        s2.unsetRecv("a", "b");
        assert.strictEqual(s2.req.get("a")?.get("b"), 0);
        assert.strictEqual(s2.reqSend.get("a")?.get("b"), 0);
      });
    });
  });
});
