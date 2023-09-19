import { assert } from "chai";
import { Client } from "../src/client.js";
import { WebSocketServer } from "ws";
import * as Message from "../src/message.js";
import { ClientData } from "../src/clientData.js";
import { Value, Text, Log } from "../src/data.js";
import { Func, AnonymousFunc } from "../src/func.js";
import { valType } from "../src/message.js";
import { View } from "../src/view.js";
import { Field, FieldBase } from "../src/field.js";
import { Member } from "../src/member.js";
import { eventType } from "../src/event.js";

describe("Client Tests", function () {
  const selfName = "test";
  let wcli: Client;
  let data: ClientData;
  let wss: WebSocketServer;
  let wssSend: (msg: Message.AnyMessage) => void;
  let wssRecv: Message.AnyMessage[];
  beforeEach(function (done) {
    wssRecv = [];
    wss = new WebSocketServer({ port: 37530 });
    wss.on("connection", (ws) => {
      ws.on("error", console.error);
      ws.on("message", (data) => {
        wssRecv = wssRecv.concat(Message.unpack(data as ArrayBuffer));
      });
      wssSend = (msg) => ws.send(Message.pack([msg]));
    });
    setTimeout(() => {
      wcli = new Client(selfName, "127.0.0.1", 37530);
      data = wcli.data;
      setTimeout(done, 10);
    }, 10);
  });
  afterEach(function (done) {
    wcli.close();
    wss.close();
    setTimeout(done, 10);
  });
  it("successfully connects", function (done) {
    assert.isTrue(wcli.connected);
    const msg = { kind: Message.kind.syncInit, M: selfName, m: 0 };
    wcli.send([msg]);
    setTimeout(() => {
      assert.deepEqual(wssRecv[0], msg);
      done();
    }, 10);
  });
  describe("#name", function () {
    it("returns self name", function () {
      assert.strictEqual(wcli.name, selfName);
    });
  });
  describe("#member()", function () {
    it("returns Member object", function () {
      const v = wcli.member("a");
      assert.instanceOf(v, Member);
      assert.strictEqual(v.name, "a");
    });
  });
  describe("#members()", function () {
    it("returns list of members in data.valueStore.entry", function () {
      data.valueStore.entry.set("a", ["b", "c", "d"]);
      assert.isArray(wcli.members());
      assert.lengthOf(wcli.members(), 1);
    });
    it("returns empty array by default", function () {
      assert.isArray(wcli.members());
      assert.isEmpty(wcli.members());
    });
  });
  describe("#onMemberEntry", function () {
    it("handles member entry event", function () {
      let called = 0;
      wcli.onMemberEntry.on(() => ++called);
      data.eventEmitter.emit(eventType.memberEntry());
    });
  });
  describe("messages", function () {
    describe("sync", function () {
      it("send SyncInit at the first #sync() call", function (done) {
        wcli.sync();
        setTimeout(() => {
          assert.deepEqual(wssRecv[0], {
            kind: Message.kind.syncInit,
            M: selfName,
            m: 0,
          });
          done();
        }, 10);
      });
      it("does not send SyncInit twice", function (done) {
        wcli.sync();
        setTimeout(() => {
          wssRecv = [];
          wcli.sync();
          setTimeout(() => {
            assert.notExists(
              wssRecv.find((m) => m.kind === Message.kind.syncInit)
            );
            done();
          }, 10);
        }, 10);
      });
      it("send Sync every time", function (done) {
        wcli.sync();
        setTimeout(() => {
          assert.exists(wssRecv.find((m) => m.kind === Message.kind.sync));
          wssRecv = [];
          wcli.sync();
          setTimeout(() => {
            assert.exists(wssRecv.find((m) => m.kind === Message.kind.sync));
            done();
          }, 10);
        }, 10);
      });
    });
    describe("receiving entry", function () {
      it("member entry", function (done) {
        let called = 0;
        wcli.onMemberEntry.on((m: Member) => {
          ++called;
          assert.strictEqual(m.name, "a");
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          assert.lengthOf(wcli.members(), 1);
          assert.strictEqual(wcli.members()[0].name, "a");
          assert.strictEqual(data.memberIds.get("a"), 10);
          done();
        }, 10);
      });
      it("value entry", function (done) {
        let called = 0;
        wcli.member("a").onValueEntry.on((v: Value) => {
          ++called;
          assert.strictEqual(v.member.name, "a");
          assert.strictEqual(v.name, "b");
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        wssSend({ kind: Message.kind.valueEntry, m: 10, f: "b" });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          assert.lengthOf(wcli.member("a").values(), 1);
          done();
        }, 10);
      });
      it("text entry", function (done) {
        let called = 0;
        wcli.member("a").onTextEntry.on((v: Text) => {
          ++called;
          assert.strictEqual(v.member.name, "a");
          assert.strictEqual(v.name, "b");
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        wssSend({ kind: Message.kind.textEntry, m: 10, f: "b" });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          assert.lengthOf(wcli.member("a").texts(), 1);
          done();
        }, 10);
      });
      it("view entry", function (done) {
        let called = 0;
        wcli.member("a").onViewEntry.on((v: View) => {
          ++called;
          assert.strictEqual(v.member.name, "a");
          assert.strictEqual(v.name, "b");
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        wssSend({ kind: Message.kind.viewEntry, m: 10, f: "b" });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          assert.lengthOf(wcli.member("a").views(), 1);
          done();
        }, 10);
      });
      it("func entry", function (done) {
        let called = 0;
        wcli.member("a").onFuncEntry.on((v: Func) => {
          ++called;
          assert.strictEqual(v.member.name, "a");
          assert.strictEqual(v.name, "b");
          assert.strictEqual(v.returnType, valType.number_);
          assert.lengthOf(v.args, 1);
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        wssSend({
          kind: Message.kind.funcInfo,
          m: 10,
          f: "b",
          r: valType.number_,
          a: [{ n: "a", t: valType.number_, i: null, m: null, x: null, o: [] }],
        });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          assert.lengthOf(wcli.member("a").funcs(), 1);
          done();
        }, 10);
      });
      it("sync", function (done) {
        let called = 0;
        wcli.member("a").onSync.on((v: Member) => {
          ++called;
          assert.strictEqual(v.name, "a");
        });
        wssSend({ kind: Message.kind.syncInit, M: "a", m: 10 });
        wssSend({ kind: Message.kind.sync, m: 10, t: 1000 });
        setTimeout(() => {
          assert.strictEqual(called, 1);
          done();
        }, 10);
      });
    });
  });
});
