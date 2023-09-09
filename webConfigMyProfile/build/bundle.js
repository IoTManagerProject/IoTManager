var app = (function () {
  "use strict";
  function t() {}
  const e = (t) => t;
  function n(t, e) {
    for (const n in e) t[n] = e[n];
    return t;
  }
  function o(t) {
    return t();
  }
  function r() {
    return Object.create(null);
  }
  function l(t) {
    t.forEach(o);
  }
  function i(t) {
    return "function" == typeof t;
  }
  function s(t, e) {
    return t != t
      ? e == e
      : t !== e || (t && "object" == typeof t) || "function" == typeof t;
  }
  function c(t, e, n, o) {
    if (t) {
      const r = a(t, e, n, o);
      return t[0](r);
    }
  }
  function a(t, e, o, r) {
    return t[1] && r ? n(o.ctx.slice(), t[1](r(e))) : o.ctx;
  }
  function u(t, e, n, o) {
    if (t[2] && o) {
      const r = t[2](o(n));
      if (void 0 === e.dirty) return r;
      if ("object" == typeof r) {
        const t = [],
          n = Math.max(e.dirty.length, r.length);
        for (let o = 0; o < n; o += 1) t[o] = e.dirty[o] | r[o];
        return t;
      }
      return e.dirty | r;
    }
    return e.dirty;
  }
  function f(t, e, n, o, r, l) {
    if (r) {
      const i = a(e, n, o, l);
      t.p(i, r);
    }
  }
  function d(t) {
    if (t.ctx.length > 32) {
      const e = [],
        n = t.ctx.length / 32;
      for (let t = 0; t < n; t++) e[t] = -1;
      return e;
    }
    return -1;
  }
  const p = "undefined" != typeof window;
  let h = p ? () => window.performance.now() : () => Date.now(),
    m = p ? (t) => requestAnimationFrame(t) : t;
  const g = new Set();
  function v(t) {
    g.forEach((e) => {
      e.c(t) || (g.delete(e), e.f());
    }),
      0 !== g.size && m(v);
  }
  function $(t) {
    let e;
    return (
      0 === g.size && m(v),
      {
        promise: new Promise((n) => {
          g.add((e = { c: t, f: n }));
        }),
        abort() {
          g.delete(e);
        },
      }
    );
  }
  function b(t, e) {
    t.appendChild(e);
  }
  function y(t) {
    if (!t) return document;
    const e = t.getRootNode ? t.getRootNode() : t.ownerDocument;
    return e && e.host ? e : t.ownerDocument;
  }
  function x(t) {
    const e = S("style");
    return (
      (function (t, e) {
        b(t.head || t, e), e.sheet;
      })(y(t), e),
      e.sheet
    );
  }
  function _(t, e, n) {
    t.insertBefore(e, n || null);
  }
  function w(t) {
    t.parentNode.removeChild(t);
  }
  function k(t, e) {
    for (let n = 0; n < t.length; n += 1) t[n] && t[n].d(e);
  }
  function S(t) {
    return document.createElement(t);
  }
  function j(t) {
    return document.createTextNode(t);
  }
  function O() {
    return j(" ");
  }
  function C() {
    return j("");
  }
  function N(t, e, n, o) {
    return t.addEventListener(e, n, o), () => t.removeEventListener(e, n, o);
  }
  function P(t, e, n) {
    null == n
      ? t.removeAttribute(e)
      : t.getAttribute(e) !== n && t.setAttribute(e, n);
  }
  function q(t, e) {
    (e = "" + e), t.wholeText !== e && (t.data = e);
  }
  function M(t, e) {
    t.value = null == e ? "" : e;
  }
  function I(t, e, n, o) {
    null === n
      ? t.style.removeProperty(e)
      : t.style.setProperty(e, n, o ? "important" : "");
  }
  function T(t, e) {
    for (let n = 0; n < t.options.length; n += 1) {
      const o = t.options[n];
      if (o.__value === e) return void (o.selected = !0);
    }
    t.selectedIndex = -1;
  }
  function E(t) {
    const e = t.querySelector(":checked") || t.options[0];
    return e && e.__value;
  }
  function R(t, e, { bubbles: n = !1, cancelable: o = !1 } = {}) {
    const r = document.createEvent("CustomEvent");
    return r.initCustomEvent(t, n, o, e), r;
  }
  const G = new Map();
  let B,
    U = 0;
  function D(t, e, n, o, r, l, i, s = 0) {
    const c = 16.666 / o;
    let a = "{\n";
    for (let t = 0; t <= 1; t += c) {
      const o = e + (n - e) * l(t);
      a += 100 * t + `%{${i(o, 1 - o)}}\n`;
    }
    const u = a + `100% {${i(n, 1 - n)}}\n}`,
      f = `__svelte_${(function (t) {
        let e = 5381,
          n = t.length;
        for (; n--; ) e = ((e << 5) - e) ^ t.charCodeAt(n);
        return e >>> 0;
      })(u)}_${s}`,
      d = y(t),
      { stylesheet: p, rules: h } =
        G.get(d) ||
        (function (t, e) {
          const n = { stylesheet: x(e), rules: {} };
          return G.set(t, n), n;
        })(d, t);
    h[f] ||
      ((h[f] = !0), p.insertRule(`@keyframes ${f} ${u}`, p.cssRules.length));
    const m = t.style.animation || "";
    return (
      (t.style.animation = `${
        m ? `${m}, ` : ""
      }${f} ${o}ms linear ${r}ms 1 both`),
      (U += 1),
      f
    );
  }
  function A(t, e) {
    const n = (t.style.animation || "").split(", "),
      o = n.filter(
        e ? (t) => t.indexOf(e) < 0 : (t) => -1 === t.indexOf("__svelte")
      ),
      r = n.length - o.length;
    r &&
      ((t.style.animation = o.join(", ")),
      (U -= r),
      U ||
        m(() => {
          U ||
            (G.forEach((t) => {
              const { ownerNode: e } = t.stylesheet;
              e && w(e);
            }),
            G.clear());
        }));
  }
  function F(n, o, r, l) {
    if (!o) return t;
    const i = n.getBoundingClientRect();
    if (
      o.left === i.left &&
      o.right === i.right &&
      o.top === i.top &&
      o.bottom === i.bottom
    )
      return t;
    const {
      delay: s = 0,
      duration: c = 300,
      easing: a = e,
      start: u = h() + s,
      end: f = u + c,
      tick: d = t,
      css: p,
    } = r(n, { from: o, to: i }, l);
    let m,
      g = !0,
      v = !1;
    function b() {
      p && A(n, m), (g = !1);
    }
    return (
      $((t) => {
        if ((!v && t >= u && (v = !0), v && t >= f && (d(1, 0), b()), !g))
          return !1;
        if (v) {
          const e = 0 + 1 * a((t - u) / c);
          d(e, 1 - e);
        }
        return !0;
      }),
      p && (m = D(n, 0, 1, c, s, a, p)),
      s || (v = !0),
      d(0, 1),
      b
    );
  }
  function L(t) {
    const e = getComputedStyle(t);
    if ("absolute" !== e.position && "fixed" !== e.position) {
      const { width: n, height: o } = e,
        r = t.getBoundingClientRect();
      (t.style.position = "absolute"),
        (t.style.width = n),
        (t.style.height = o),
        J(t, r);
    }
  }
  function J(t, e) {
    const n = t.getBoundingClientRect();
    if (e.left !== n.left || e.top !== n.top) {
      const o = getComputedStyle(t),
        r = "none" === o.transform ? "" : o.transform;
      t.style.transform = `${r} translate(${e.left - n.left}px, ${
        e.top - n.top
      }px)`;
    }
  }
  function V(t) {
    B = t;
  }
  function z() {
    if (!B) throw new Error("Function called outside component initialization");
    return B;
  }
  const K = [],
    H = [],
    Q = [],
    W = [],
    X = Promise.resolve();
  let Y = !1;
  function Z(t) {
    Q.push(t);
  }
  function tt(t) {
    W.push(t);
  }
  const et = new Set();
  let nt,
    ot = 0;
  function rt() {
    const t = B;
    do {
      for (; ot < K.length; ) {
        const t = K[ot];
        ot++, V(t), lt(t.$$);
      }
      for (V(null), K.length = 0, ot = 0; H.length; ) H.pop()();
      for (let t = 0; t < Q.length; t += 1) {
        const e = Q[t];
        et.has(e) || (et.add(e), e());
      }
      Q.length = 0;
    } while (K.length);
    for (; W.length; ) W.pop()();
    (Y = !1), et.clear(), V(t);
  }
  function lt(t) {
    if (null !== t.fragment) {
      t.update(), l(t.before_update);
      const e = t.dirty;
      (t.dirty = [-1]),
        t.fragment && t.fragment.p(t.ctx, e),
        t.after_update.forEach(Z);
    }
  }
  function it() {
    return (
      nt ||
        ((nt = Promise.resolve()),
        nt.then(() => {
          nt = null;
        })),
      nt
    );
  }
  function st(t, e, n) {
    t.dispatchEvent(R(`${e ? "intro" : "outro"}${n}`));
  }
  const ct = new Set();
  let at;
  function ut() {
    at = { r: 0, c: [], p: at };
  }
  function ft() {
    at.r || l(at.c), (at = at.p);
  }
  function dt(t, e) {
    t && t.i && (ct.delete(t), t.i(e));
  }
  function pt(t, e, n, o) {
    if (t && t.o) {
      if (ct.has(t)) return;
      ct.add(t),
        at.c.push(() => {
          ct.delete(t), o && (n && t.d(1), o());
        }),
        t.o(e);
    } else o && o();
  }
  const ht = { duration: 0 };
  function mt(n, o, r) {
    let l,
      s,
      c = o(n, r),
      a = !1,
      u = 0;
    function f() {
      l && A(n, l);
    }
    function d() {
      const {
        delay: o = 0,
        duration: r = 300,
        easing: i = e,
        tick: d = t,
        css: p,
      } = c || ht;
      p && (l = D(n, 0, 1, r, o, i, p, u++)), d(0, 1);
      const m = h() + o,
        g = m + r;
      s && s.abort(),
        (a = !0),
        Z(() => st(n, !0, "start")),
        (s = $((t) => {
          if (a) {
            if (t >= g) return d(1, 0), st(n, !0, "end"), f(), (a = !1);
            if (t >= m) {
              const e = i((t - m) / r);
              d(e, 1 - e);
            }
          }
          return a;
        }));
    }
    let p = !1;
    return {
      start() {
        p || ((p = !0), A(n), i(c) ? ((c = c()), it().then(d)) : d());
      },
      invalidate() {
        p = !1;
      },
      end() {
        a && (f(), (a = !1));
      },
    };
  }
  function gt(n, o, r) {
    let s,
      c = o(n, r),
      a = !0;
    const u = at;
    function f() {
      const {
        delay: o = 0,
        duration: r = 300,
        easing: i = e,
        tick: f = t,
        css: d,
      } = c || ht;
      d && (s = D(n, 1, 0, r, o, i, d));
      const p = h() + o,
        m = p + r;
      Z(() => st(n, !1, "start")),
        $((t) => {
          if (a) {
            if (t >= m) return f(0, 1), st(n, !1, "end"), --u.r || l(u.c), !1;
            if (t >= p) {
              const e = i((t - p) / r);
              f(1 - e, e);
            }
          }
          return a;
        });
    }
    return (
      (u.r += 1),
      i(c)
        ? it().then(() => {
            (c = c()), f();
          })
        : f(),
      {
        end(t) {
          t && c.tick && c.tick(1, 0), a && (s && A(n, s), (a = !1));
        },
      }
    );
  }
  function vt(t, e) {
    t.f(),
      (function (t, e) {
        pt(t, 1, 1, () => {
          e.delete(t.key);
        });
      })(t, e);
  }
  function $t(t, e, n, o, r, l, i, s, c, a, u, f) {
    let d = t.length,
      p = l.length,
      h = d;
    const m = {};
    for (; h--; ) m[t[h].key] = h;
    const g = [],
      v = new Map(),
      $ = new Map();
    for (h = p; h--; ) {
      const t = f(r, l, h),
        s = n(t);
      let c = i.get(s);
      c ? o && c.p(t, e) : ((c = a(s, t)), c.c()),
        v.set(s, (g[h] = c)),
        s in m && $.set(s, Math.abs(h - m[s]));
    }
    const b = new Set(),
      y = new Set();
    function x(t) {
      dt(t, 1), t.m(s, u), i.set(t.key, t), (u = t.first), p--;
    }
    for (; d && p; ) {
      const e = g[p - 1],
        n = t[d - 1],
        o = e.key,
        r = n.key;
      e === n
        ? ((u = e.first), d--, p--)
        : v.has(r)
        ? !i.has(o) || b.has(o)
          ? x(e)
          : y.has(r)
          ? d--
          : $.get(o) > $.get(r)
          ? (y.add(o), x(e))
          : (b.add(r), d--)
        : (c(n, i), d--);
    }
    for (; d--; ) {
      const e = t[d];
      v.has(e.key) || c(e, i);
    }
    for (; p; ) x(g[p - 1]);
    return g;
  }
  function bt(t, e, n) {
    const o = t.$$.props[e];
    void 0 !== o && ((t.$$.bound[o] = n), n(t.$$.ctx[o]));
  }
  function yt(t) {
    t && t.c();
  }
  function xt(t, e, n, r) {
    const { fragment: s, on_mount: c, on_destroy: a, after_update: u } = t.$$;
    s && s.m(e, n),
      r ||
        Z(() => {
          const e = c.map(o).filter(i);
          a ? a.push(...e) : l(e), (t.$$.on_mount = []);
        }),
      u.forEach(Z);
  }
  function _t(t, e) {
    const n = t.$$;
    null !== n.fragment &&
      (l(n.on_destroy),
      n.fragment && n.fragment.d(e),
      (n.on_destroy = n.fragment = null),
      (n.ctx = []));
  }
  function wt(t, e) {
    -1 === t.$$.dirty[0] &&
      (K.push(t), Y || ((Y = !0), X.then(rt)), t.$$.dirty.fill(0)),
      (t.$$.dirty[(e / 31) | 0] |= 1 << e % 31);
  }
  function kt(e, n, o, i, s, c, a, u = [-1]) {
    const f = B;
    V(e);
    const d = (e.$$ = {
      fragment: null,
      ctx: null,
      props: c,
      update: t,
      not_equal: s,
      bound: r(),
      on_mount: [],
      on_destroy: [],
      on_disconnect: [],
      before_update: [],
      after_update: [],
      context: new Map(n.context || (f ? f.$$.context : [])),
      callbacks: r(),
      dirty: u,
      skip_bound: !1,
      root: n.target || f.$$.root,
    });
    a && a(d.root);
    let p = !1;
    if (
      ((d.ctx = o
        ? o(e, n.props || {}, (t, n, ...o) => {
            const r = o.length ? o[0] : n;
            return (
              d.ctx &&
                s(d.ctx[t], (d.ctx[t] = r)) &&
                (!d.skip_bound && d.bound[t] && d.bound[t](r), p && wt(e, t)),
              n
            );
          })
        : []),
      d.update(),
      (p = !0),
      l(d.before_update),
      (d.fragment = !!i && i(d.ctx)),
      n.target)
    ) {
      if (n.hydrate) {
        const t = (function (t) {
          return Array.from(t.childNodes);
        })(n.target);
        d.fragment && d.fragment.l(t), t.forEach(w);
      } else d.fragment && d.fragment.c();
      n.intro && dt(e.$$.fragment),
        xt(e, n.target, n.anchor, n.customElement),
        rt();
    }
    V(f);
  }
  class St {
    $destroy() {
      _t(this, 1), (this.$destroy = t);
    }
    $on(t, e) {
      const n = this.$$.callbacks[t] || (this.$$.callbacks[t] = []);
      return (
        n.push(e),
        () => {
          const t = n.indexOf(e);
          -1 !== t && n.splice(t, 1);
        }
      );
    }
    $set(t) {
      var e;
      this.$$set &&
        ((e = t), 0 !== Object.keys(e).length) &&
        ((this.$$.skip_bound = !0), this.$$set(t), (this.$$.skip_bound = !1));
    }
  }
  function jt(t) {
    const e = t - 1;
    return e * e * e + 1;
  }
  function Ot(t) {
    return --t * t * t * t * t + 1;
  }
  /*! *****************************************************************************
    Copyright (c) Microsoft Corporation.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
    REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
    LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
    OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.
    ***************************************************************************** */ function Ct(
    t
  ) {
    var { fallback: e } = t,
      o = (function (t, e) {
        var n = {};
        for (var o in t)
          Object.prototype.hasOwnProperty.call(t, o) &&
            e.indexOf(o) < 0 &&
            (n[o] = t[o]);
        if (null != t && "function" == typeof Object.getOwnPropertySymbols) {
          var r = 0;
          for (o = Object.getOwnPropertySymbols(t); r < o.length; r++)
            e.indexOf(o[r]) < 0 &&
              Object.prototype.propertyIsEnumerable.call(t, o[r]) &&
              (n[o[r]] = t[o[r]]);
        }
        return n;
      })(t, ["fallback"]);
    const r = new Map(),
      l = new Map();
    function s(t, r, l) {
      return (s, c) => (
        t.set(c.key, { rect: s.getBoundingClientRect() }),
        () => {
          if (r.has(c.key)) {
            const { rect: t } = r.get(c.key);
            return (
              r.delete(c.key),
              (function (t, e, r) {
                const {
                    delay: l = 0,
                    duration: s = (t) => 30 * Math.sqrt(t),
                    easing: c = jt,
                  } = n(n({}, o), r),
                  a = e.getBoundingClientRect(),
                  u = t.left - a.left,
                  f = t.top - a.top,
                  d = t.width / a.width,
                  p = t.height / a.height,
                  h = Math.sqrt(u * u + f * f),
                  m = getComputedStyle(e),
                  g = "none" === m.transform ? "" : m.transform,
                  v = +m.opacity;
                return {
                  delay: l,
                  duration: i(s) ? s(h) : s,
                  easing: c,
                  css: (t, e) =>
                    `\n\t\t\t\topacity: ${
                      t * v
                    };\n\t\t\t\ttransform-origin: top left;\n\t\t\t\ttransform: ${g} translate(${
                      e * u
                    }px,${e * f}px) scale(${t + (1 - t) * d}, ${
                      t + (1 - t) * p
                    });\n\t\t\t`,
                };
              })(t, s, c)
            );
          }
          return t.delete(c.key), e && e(s, c, l);
        }
      );
    }
    return [s(l, r, !1), s(r, l, !0)];
  }
  function Nt(t, { from: e, to: n }, o = {}) {
    const r = getComputedStyle(t),
      l = "none" === r.transform ? "" : r.transform,
      [s, c] = r.transformOrigin.split(" ").map(parseFloat),
      a = e.left + (e.width * s) / n.width - (n.left + s),
      u = e.top + (e.height * c) / n.height - (n.top + c),
      {
        delay: f = 0,
        duration: d = (t) => 120 * Math.sqrt(t),
        easing: p = jt,
      } = o;
    return {
      delay: f,
      duration: i(d) ? d(Math.sqrt(a * a + u * u)) : d,
      easing: p,
      css: (t, o) => {
        const r = o * a,
          i = o * u,
          s = t + (o * e.width) / n.width,
          c = t + (o * e.height) / n.height;
        return `transform: ${l} translate(${r}px, ${i}px) scale(${s}, ${c});`;
      },
    };
  }
  function Pt(e) {
    let n, o, r, l, i, s, c, a, u, f, d;
    return {
      c() {
        (n = S("div")),
          (o = S("div")),
          (r = S("div")),
          (l = O()),
          (i = S("div")),
          (s = O()),
          (c = S("div")),
          (a = O()),
          (u = S("div")),
          (f = j(e[0])),
          (d = j("%")),
          P(r, "class", "gauge-fill gauge-bg svelte-1er7lpr"),
          P(i, "class", "gauge-fill svelte-1er7lpr"),
          I(i, "transform", "rotate(-" + e[2] + "deg)"),
          I(i, "box-shadow", "inset 0 0 60px -5px " + e[1]),
          P(c, "class", "gauge-white svelte-1er7lpr"),
          P(u, "class", "gauge-value svelte-1er7lpr"),
          P(o, "class", "gauge-overflow svelte-1er7lpr"),
          P(n, "class", "gauge svelte-1er7lpr");
      },
      m(t, e) {
        _(t, n, e),
          b(n, o),
          b(o, r),
          b(o, l),
          b(o, i),
          b(o, s),
          b(o, c),
          b(o, a),
          b(o, u),
          b(u, f),
          b(u, d);
      },
      p(t, [e]) {
        4 & e && I(i, "transform", "rotate(-" + t[2] + "deg)"),
          2 & e && I(i, "box-shadow", "inset 0 0 60px -5px " + t[1]),
          1 & e && q(f, t[0]);
      },
      i: t,
      o: t,
      d(t) {
        t && w(n);
      },
    };
  }
  function qt(t, e, n) {
    let o,
      r,
      { title: l } = e,
      { value: i = l } = e,
      { thresholds: s = "90,100" } = e,
      { colors: c = "#7CFC00,#FFD700,#FF0000" } = e;
    return (
      (t.$$set = (t) => {
        "title" in t && n(3, (l = t.title)),
          "value" in t && n(0, (i = t.value)),
          "thresholds" in t && n(4, (s = t.thresholds)),
          "colors" in t && n(5, (c = t.colors));
      }),
      (t.$$.update = () => {
        if (49 & t.$$.dirty) {
          const t = String(s).split(","),
            e = t.indexOf(t.find((t) => i < parseInt(t, 10))),
            r = String(c).split(",");
          n(1, (o = r[e] || r[r.length - 1]));
        }
        1 & t.$$.dirty && n(2, (r = 1.8 * (100 - i)));
      }),
      [i, o, r, l, s, c]
    );
  }
  class Mt extends St {
    constructor(t) {
      super(),
        kt(this, t, qt, Pt, s, {
          title: 3,
          value: 0,
          thresholds: 4,
          colors: 5,
        });
    }
  }
  const It = (t) => ({}),
    Tt = (t) => ({});
  function Et(t) {
    let e, n, o, r, i, s, a, p, h, m, g, v, $;
    const y = t[4].header,
      x = c(y, t, t[3], Tt),
      k = t[4].default,
      j = c(k, t, t[3], null);
    return {
      c() {
        (e = S("div")),
          (n = O()),
          (o = S("div")),
          x && x.c(),
          (r = O()),
          (i = S("hr")),
          (s = O()),
          j && j.c(),
          (a = O()),
          (p = S("hr")),
          (h = O()),
          (m = S("button")),
          (m.textContent = "Close"),
          P(e, "class", "modal-background svelte-5a6h72"),
          I(m, "float", "right"),
          I(m, "margin-right", "2%"),
          I(m, "border", "1px solid lightblue"),
          I(m, "border-radius", "8px"),
          (m.autofocus = !0),
          P(m, "class", "svelte-5a6h72"),
          P(o, "class", "modal svelte-5a6h72"),
          P(o, "role", "dialog"),
          P(o, "aria-modal", "true");
      },
      m(l, c) {
        _(l, e, c),
          _(l, n, c),
          _(l, o, c),
          x && x.m(o, null),
          b(o, r),
          b(o, i),
          b(o, s),
          j && j.m(o, null),
          b(o, a),
          b(o, p),
          b(o, h),
          b(o, m),
          t[5](o),
          (g = !0),
          m.focus(),
          v ||
            (($ = [
              N(window, "keydown", t[2]),
              N(e, "click", t[1]),
              N(m, "click", t[1]),
            ]),
            (v = !0));
      },
      p(t, [e]) {
        x &&
          x.p &&
          (!g || 8 & e) &&
          f(x, y, t, t[3], g ? u(y, t[3], e, It) : d(t[3]), Tt),
          j &&
            j.p &&
            (!g || 8 & e) &&
            f(j, k, t, t[3], g ? u(k, t[3], e, null) : d(t[3]), null);
      },
      i(t) {
        g || (dt(x, t), dt(j, t), (g = !0));
      },
      o(t) {
        pt(x, t), pt(j, t), (g = !1);
      },
      d(r) {
        r && w(e),
          r && w(n),
          r && w(o),
          x && x.d(r),
          j && j.d(r),
          t[5](null),
          (v = !1),
          l($);
      },
    };
  }
  function Rt(t, e, n) {
    let { $$slots: o = {}, $$scope: r } = e;
    const l = (function () {
        const t = z();
        return (e, n, { cancelable: o = !1 } = {}) => {
          const r = t.$$.callbacks[e];
          if (r) {
            const l = R(e, n, { cancelable: o });
            return (
              r.slice().forEach((e) => {
                e.call(t, l);
              }),
              !l.defaultPrevented
            );
          }
          return !0;
        };
      })(),
      i = () => l("close");
    let s;
    const c = "undefined" != typeof document && document.activeElement;
    var a;
    return (
      c &&
        ((a = () => {
          c.focus();
        }),
        z().$$.on_destroy.push(a)),
      (t.$$set = (t) => {
        "$$scope" in t && n(3, (r = t.$$scope));
      }),
      [
        s,
        i,
        (t) => {
          if ("Escape" !== t.key) {
            if ("Tab" === t.key) {
              const e = s.querySelectorAll("*"),
                n = Array.from(e).filter((t) => t.tabIndex >= 0);
              let o = n.indexOf(document.activeElement);
              -1 === o && t.shiftKey && (o = 0),
                (o += n.length + (t.shiftKey ? -1 : 1)),
                (o %= n.length),
                n[o].focus(),
                t.preventDefault();
            }
          } else i();
        },
        r,
        o,
        function (t) {
          H[t ? "unshift" : "push"](() => {
            (s = t), n(0, s);
          });
        },
      ]
    );
  }
  class Gt extends St {
    constructor(t) {
      super(), kt(this, t, Rt, Et, s, {});
    }
  }
  function Bt(t) {
    let e, n;
    const o = t[1].default,
      r = c(o, t, t[0], null);
    return {
      c() {
        (e = S("div")), r && r.c(), P(e, "class", "box svelte-1q88uwi");
      },
      m(t, o) {
        _(t, e, o), r && r.m(e, null), (n = !0);
      },
      p(t, [e]) {
        r &&
          r.p &&
          (!n || 1 & e) &&
          f(r, o, t, t[0], n ? u(o, t[0], e, null) : d(t[0]), null);
      },
      i(t) {
        n || (dt(r, t), (n = !0));
      },
      o(t) {
        pt(r, t), (n = !1);
      },
      d(t) {
        t && w(e), r && r.d(t);
      },
    };
  }
  function Ut(t, e, n) {
    let { $$slots: o = {}, $$scope: r } = e;
    return (
      (t.$$set = (t) => {
        "$$scope" in t && n(0, (r = t.$$scope));
      }),
      [r, o]
    );
  }
  class Dt extends St {
    constructor(t) {
      super(), kt(this, t, Ut, Bt, s, {});
    }
  }
  function At(t, e, n) {
    const o = t.slice();
    return (o[62] = e[n]), o;
  }
  function Ft(t, e, n) {
    const o = t.slice();
    return (o[65] = e[n]), o;
  }
  function Lt(t, e, n) {
    const o = t.slice();
    return (o[68] = e[n][0]), (o[69] = e[n][1]), o;
  }
  function Jt(t, e, n) {
    const o = t.slice();
    return (o[68] = e[n][0]), (o[69] = e[n][1]), o;
  }
  function Vt(t, e, n) {
    const o = t.slice();
    return (o[74] = e[n]), (o[75] = e), (o[76] = n), o;
  }
  function zt(t, e, n) {
    const o = t.slice();
    return (o[68] = e[n][0]), (o[69] = e[n][1]), o;
  }
  function Kt(t, e, n) {
    const o = t.slice();
    return (o[74] = e[n]), (o[79] = e), (o[80] = n), o;
  }
  function Ht(t, e, n) {
    const o = t.slice();
    return (o[81] = e[n]), o;
  }
  function Qt(t) {
    let e, n, o, r;
    function l(e) {
      t[19](e);
    }
    let i = {};
    return (
      void 0 !== t[1] && (i.value = t[1]),
      (e = new Mt({ props: i })),
      H.push(() => bt(e, "value", l)),
      {
        c() {
          yt(e.$$.fragment), (o = j("                   "));
        },
        m(t, n) {
          xt(e, t, n), _(t, o, n), (r = !0);
        },
        p(t, o) {
          const r = {};
          !n && 2 & o[0] && ((n = !0), (r.value = t[1]), tt(() => (n = !1))),
            e.$set(r);
        },
        i(t) {
          r || (dt(e.$$.fragment, t), (r = !0));
        },
        o(t) {
          pt(e.$$.fragment, t), (r = !1);
        },
        d(t) {
          _t(e, t), t && w(o);
        },
      }
    );
  }
  function Wt(t) {
    let e, n, o, r;
    function l(e) {
      t[20](e);
    }
    let i = {};
    return (
      void 0 !== t[1] && (i.value = t[1]),
      (e = new Mt({ props: i })),
      H.push(() => bt(e, "value", l)),
      {
        c() {
          yt(e.$$.fragment), (o = j("                   "));
        },
        m(t, n) {
          xt(e, t, n), _(t, o, n), (r = !0);
        },
        p(t, o) {
          const r = {};
          !n && 2 & o[0] && ((n = !0), (r.value = t[1]), tt(() => (n = !1))),
            e.$set(r);
        },
        i(t) {
          r || (dt(e.$$.fragment, t), (r = !0));
        },
        o(t) {
          pt(e.$$.fragment, t), (r = !1);
        },
        d(t) {
          _t(e, t), t && w(o);
        },
      }
    );
  }
  function Xt(t) {
    let e, n, o, r;
    function l(e) {
      t[21](e);
    }
    let i = {};
    return (
      void 0 !== t[1] && (i.value = t[1]),
      (e = new Mt({ props: i })),
      H.push(() => bt(e, "value", l)),
      {
        c() {
          yt(e.$$.fragment), (o = j("                   "));
        },
        m(t, n) {
          xt(e, t, n), _(t, o, n), (r = !0);
        },
        p(t, o) {
          const r = {};
          !n && 2 & o[0] && ((n = !0), (r.value = t[1]), tt(() => (n = !1))),
            e.$set(r);
        },
        i(t) {
          r || (dt(e.$$.fragment, t), (r = !0));
        },
        o(t) {
          pt(e.$$.fragment, t), (r = !1);
        },
        d(t) {
          _t(e, t), t && w(o);
        },
      }
    );
  }
  function Yt(t) {
    let e, n, o, r;
    function l(e) {
      t[22](e);
    }
    let i = {};
    return (
      void 0 !== t[1] && (i.value = t[1]),
      (e = new Mt({ props: i })),
      H.push(() => bt(e, "value", l)),
      {
        c() {
          yt(e.$$.fragment), (o = j("                   "));
        },
        m(t, n) {
          xt(e, t, n), _(t, o, n), (r = !0);
        },
        p(t, o) {
          const r = {};
          !n && 2 & o[0] && ((n = !0), (r.value = t[1]), tt(() => (n = !1))),
            e.$set(r);
        },
        i(t) {
          r || (dt(e.$$.fragment, t), (r = !0));
        },
        o(t) {
          pt(e.$$.fragment, t), (r = !1);
        },
        d(t) {
          _t(e, t), t && w(o);
        },
      }
    );
  }
  function Zt(e) {
    let n,
      o,
      r,
      l = e[81].text + "";
    return {
      c() {
        (n = S("option")),
          (o = j(l)),
          (r = O()),
          (n.__value = e[81]),
          (n.value = n.__value);
      },
      m(t, e) {
        _(t, n, e), b(n, o), b(n, r);
      },
      p: t,
      d(t) {
        t && w(n);
      },
    };
  }
  function te(e) {
    let n,
      o,
      r,
      l = e[81].text + "";
    return {
      c() {
        (n = S("option")),
          (o = j(l)),
          (r = O()),
          (n.__value = e[81]),
          (n.value = n.__value);
      },
      m(t, e) {
        _(t, n, e), b(n, o), b(n, r);
      },
      p: t,
      d(t) {
        t && w(n);
      },
    };
  }
  function ee(e) {
    let n,
      o,
      r,
      l = e[81].text + "";
    return {
      c() {
        (n = S("option")),
          (o = j(l)),
          (r = O()),
          (n.__value = e[81]),
          (n.value = n.__value);
      },
      m(t, e) {
        _(t, n, e), b(n, o), b(n, r);
      },
      p: t,
      d(t) {
        t && w(n);
      },
    };
  }
  function ne(t) {
    let e;
    let n = (function (t, e) {
        return "esp8266_1mb" == t[81].text
          ? ee
          : "esp8266_1mb_ota" == t[81].text
          ? te
          : Zt;
      })(t),
      o = n(t);
    return {
      c() {
        o.c(), (e = C());
      },
      m(t, n) {
        o.m(t, n), _(t, e, n);
      },
      p(t, e) {
        o.p(t, e);
      },
      d(t) {
        o.d(t), t && w(e);
      },
    };
  }
  function oe(t) {
    let e,
      n,
      o,
      r,
      i,
      s,
      c,
      a,
      u,
      f,
      d,
      p,
      h,
      m,
      g,
      v,
      $,
      y,
      x,
      k,
      C,
      q,
      T,
      E,
      R,
      G,
      B,
      U,
      D,
      A,
      F,
      L,
      J,
      V,
      z,
      K,
      H,
      Q;
    return {
      c() {
        (e = S("p")),
          (n = j("wifi сеть       ")),
          (o = S("input")),
          (r = S("br")),
          (i = O()),
          (s = S("p")),
          (c = j("пароль       ")),
          (a = S("input")),
          (u = S("br")),
          (f = O()),
          (d = S("p")),
          (p = j("MQTT сервер       ")),
          (h = S("input")),
          (m = S("br")),
          (g = O()),
          (v = S("p")),
          ($ = j("порт       ")),
          (y = S("input")),
          (x = S("br")),
          (k = O()),
          (C = S("p")),
          (q = j("префикс       ")),
          (T = S("input")),
          (E = S("br")),
          (R = O()),
          (G = S("p")),
          (B = j("логин       ")),
          (U = S("input")),
          (D = S("br")),
          (A = O()),
          (F = S("p")),
          (L = j("пароль       ")),
          (J = S("input")),
          (V = S("br")),
          (z = O()),
          (K = S("br")),
          I(o, "float", "right"),
          P(o, "class", "svelte-185rejv"),
          I(e, "color", "#ccc"),
          I(a, "float", "right"),
          P(a, "class", "svelte-185rejv"),
          I(s, "color", "#ccc"),
          I(h, "float", "right"),
          P(h, "class", "svelte-185rejv"),
          I(d, "color", "#ccc"),
          I(y, "float", "right"),
          P(y, "class", "svelte-185rejv"),
          I(v, "color", "#ccc"),
          I(T, "float", "right"),
          P(T, "class", "svelte-185rejv"),
          I(C, "color", "#ccc"),
          I(U, "float", "right"),
          P(U, "class", "svelte-185rejv"),
          I(G, "color", "#ccc"),
          I(J, "float", "right"),
          P(J, "class", "svelte-185rejv"),
          I(F, "color", "#ccc");
      },
      m(l, w) {
        _(l, e, w),
          b(e, n),
          b(e, o),
          M(o, t[0].iotmSettings.routerssid),
          b(e, r),
          _(l, i, w),
          _(l, s, w),
          b(s, c),
          b(s, a),
          M(a, t[0].iotmSettings.routerpass),
          b(s, u),
          _(l, f, w),
          _(l, d, w),
          b(d, p),
          b(d, h),
          M(h, t[0].iotmSettings.mqttServer),
          b(d, m),
          _(l, g, w),
          _(l, v, w),
          b(v, $),
          b(v, y),
          M(y, t[0].iotmSettings.mqttPort),
          b(v, x),
          _(l, k, w),
          _(l, C, w),
          b(C, q),
          b(C, T),
          M(T, t[0].iotmSettings.mqttPrefix),
          b(C, E),
          _(l, R, w),
          _(l, G, w),
          b(G, B),
          b(G, U),
          M(U, t[0].iotmSettings.mqttUser),
          b(G, D),
          _(l, A, w),
          _(l, F, w),
          b(F, L),
          b(F, J),
          M(J, t[0].iotmSettings.mqttPass),
          b(F, V),
          _(l, z, w),
          _(l, K, w),
          H ||
            ((Q = [
              N(o, "input", t[29]),
              N(a, "input", t[30]),
              N(h, "input", t[31]),
              N(y, "input", t[32]),
              N(T, "input", t[33]),
              N(U, "input", t[34]),
              N(J, "input", t[35]),
            ]),
            (H = !0));
      },
      p(t, e) {
        1 & e[0] &&
          o.value !== t[0].iotmSettings.routerssid &&
          M(o, t[0].iotmSettings.routerssid),
          1 & e[0] &&
            a.value !== t[0].iotmSettings.routerpass &&
            M(a, t[0].iotmSettings.routerpass),
          1 & e[0] &&
            h.value !== t[0].iotmSettings.mqttServer &&
            M(h, t[0].iotmSettings.mqttServer),
          1 & e[0] &&
            y.value !== t[0].iotmSettings.mqttPort &&
            M(y, t[0].iotmSettings.mqttPort),
          1 & e[0] &&
            T.value !== t[0].iotmSettings.mqttPrefix &&
            M(T, t[0].iotmSettings.mqttPrefix),
          1 & e[0] &&
            U.value !== t[0].iotmSettings.mqttUser &&
            M(U, t[0].iotmSettings.mqttUser),
          1 & e[0] &&
            J.value !== t[0].iotmSettings.mqttPass &&
            M(J, t[0].iotmSettings.mqttPass);
      },
      d(t) {
        t && w(e),
          t && w(i),
          t && w(s),
          t && w(f),
          t && w(d),
          t && w(g),
          t && w(v),
          t && w(k),
          t && w(C),
          t && w(R),
          t && w(G),
          t && w(A),
          t && w(F),
          t && w(z),
          t && w(K),
          (H = !1),
          l(Q);
      },
    };
  }
  function re(t) {
    let e,
      n,
      o,
      r,
      i,
      s,
      c,
      a,
      u,
      f,
      d,
      p,
      h,
      m,
      g,
      v,
      $,
      y,
      x,
      C,
      q,
      E,
      R,
      G,
      B,
      U,
      D,
      A,
      F,
      L = "esp8266_4mb" == t[9].text && Qt(t),
      J = "esp32_4mb" == t[9].text && Wt(t),
      V = "esp8266_1mb" == t[9].text && Xt(t),
      z = "esp8266_1mb_ota" == t[9].text && Yt(t),
      K = t[16],
      H = [];
    for (let e = 0; e < K.length; e += 1) H[e] = ne(Ht(t, K, e));
    let Q = t[8] && oe(t);
    return {
      c() {
        (e = S("form")),
          L && L.c(),
          (n = O()),
          J && J.c(),
          (o = O()),
          V && V.c(),
          (r = O()),
          z && z.c(),
          (i = O()),
          (s = S("select")),
          (c = S("option")),
          (c.textContent = "ver4dev"),
          (a = S("option")),
          (a.textContent = "stabile"),
          (u = O()),
          (f = S("select"));
        for (let t = 0; t < H.length; t += 1) H[t].c();
        (d = O()),
          (p = S("br")),
          (h = O()),
          (m = S("p")),
          (g = j("⏬")),
          (v = O()),
          ($ = S("p")),
          (y = j("⏫")),
          (x = O()),
          Q && Q.c(),
          (C = O()),
          (q = S("form")),
          (E = S("input")),
          (R = O()),
          (G = S("p")),
          (B = O()),
          (U = S("input")),
          (c.__value = "dev"),
          (c.value = c.__value),
          (a.__value = "stabile"),
          (a.value = a.__value),
          P(s, "align", "right"),
          void 0 === t[7] && Z(() => t[23].call(s)),
          P(f, "align", "right"),
          void 0 === t[9] && Z(() => t[25].call(f)),
          P(e, "align", "right"),
          I(m, "display", t[8] ? "none" : "inline"),
          I($, "display", t[8] ? "inline" : "none"),
          (E.hidden = !0),
          P(E, "type", "text"),
          P(E, "name", "configpost"),
          P(E, "class", "svelte-185rejv"),
          P(G, "align", "center"),
          P(q, "action", "index.php"),
          P(q, "method", "post"),
          P(q, "name", "fwbuilder"),
          P(U, "type", "submit"),
          I(U, "border", "1px solid lightblue"),
          I(U, "border-radius", "8px"),
          P(U, "name", "send"),
          (U.value = "Сохранить myProfile.json"),
          P(U, "class", "svelte-185rejv");
      },
      m(l, w) {
        _(l, e, w),
          L && L.m(e, null),
          b(e, n),
          J && J.m(e, null),
          b(e, o),
          V && V.m(e, null),
          b(e, r),
          z && z.m(e, null),
          b(e, i),
          b(e, s),
          b(s, c),
          b(s, a),
          T(s, t[7]),
          b(e, u),
          b(e, f);
        for (let t = 0; t < H.length; t += 1) H[t].m(f, null);
        T(f, t[9]),
          _(l, d, w),
          _(l, p, w),
          _(l, h, w),
          _(l, m, w),
          b(m, g),
          _(l, v, w),
          _(l, $, w),
          b($, y),
          _(l, x, w),
          Q && Q.m(l, w),
          _(l, C, w),
          _(l, q, w),
          b(q, E),
          M(E, t[0]),
          b(q, R),
          b(q, G),
          _(l, B, w),
          _(l, U, w),
          (D = !0),
          A ||
            ((F = [
              N(s, "change", t[23]),
              N(s, "change", t[24]),
              N(f, "change", t[25]),
              N(f, "change", t[26]),
              N(m, "click", t[27]),
              N($, "click", t[28]),
              N(E, "input", t[36]),
              N(U, "click", t[37]),
            ]),
            (A = !0));
      },
      p(t, l) {
        if (
          ("esp8266_4mb" == t[9].text
            ? L
              ? (L.p(t, l), 512 & l[0] && dt(L, 1))
              : ((L = Qt(t)), L.c(), dt(L, 1), L.m(e, n))
            : L &&
              (ut(),
              pt(L, 1, 1, () => {
                L = null;
              }),
              ft()),
          "esp32_4mb" == t[9].text
            ? J
              ? (J.p(t, l), 512 & l[0] && dt(J, 1))
              : ((J = Wt(t)), J.c(), dt(J, 1), J.m(e, o))
            : J &&
              (ut(),
              pt(J, 1, 1, () => {
                J = null;
              }),
              ft()),
          "esp8266_1mb" == t[9].text
            ? V
              ? (V.p(t, l), 512 & l[0] && dt(V, 1))
              : ((V = Xt(t)), V.c(), dt(V, 1), V.m(e, r))
            : V &&
              (ut(),
              pt(V, 1, 1, () => {
                V = null;
              }),
              ft()),
          "esp8266_1mb_ota" == t[9].text
            ? z
              ? (z.p(t, l), 512 & l[0] && dt(z, 1))
              : ((z = Yt(t)), z.c(), dt(z, 1), z.m(e, i))
            : z &&
              (ut(),
              pt(z, 1, 1, () => {
                z = null;
              }),
              ft()),
          128 & l[0] && T(s, t[7]),
          65536 & l[0])
        ) {
          let e;
          for (K = t[16], e = 0; e < K.length; e += 1) {
            const n = Ht(t, K, e);
            H[e] ? H[e].p(n, l) : ((H[e] = ne(n)), H[e].c(), H[e].m(f, null));
          }
          for (; e < H.length; e += 1) H[e].d(1);
          H.length = K.length;
        }
        66048 & l[0] && T(f, t[9]),
          (!D || 256 & l[0]) && I(m, "display", t[8] ? "none" : "inline"),
          (!D || 256 & l[0]) && I($, "display", t[8] ? "inline" : "none"),
          t[8]
            ? Q
              ? Q.p(t, l)
              : ((Q = oe(t)), Q.c(), Q.m(C.parentNode, C))
            : Q && (Q.d(1), (Q = null)),
          1 & l[0] && E.value !== t[0] && M(E, t[0]);
      },
      i(t) {
        D || (dt(L), dt(J), dt(V), dt(z), (D = !0));
      },
      o(t) {
        pt(L), pt(J), pt(V), pt(z), (D = !1);
      },
      d(t) {
        t && w(e),
          L && L.d(),
          J && J.d(),
          V && V.d(),
          z && z.d(),
          k(H, t),
          t && w(d),
          t && w(p),
          t && w(h),
          t && w(m),
          t && w(v),
          t && w($),
          t && w(x),
          Q && Q.d(t),
          t && w(C),
          t && w(q),
          t && w(B),
          t && w(U),
          (A = !1),
          l(F);
      },
    };
  }
  function le(e, n) {
    let o,
      r,
      i,
      s,
      c,
      a,
      u,
      f,
      d,
      p,
      h,
      m,
      g,
      v =
        n[74].path.slice(n[74].path.lastIndexOf("/") + 1, n[74].path.length) +
        "",
      $ = t;
    function y() {
      n[38].call(r, n[79], n[80]);
    }
    function x() {
      return n[39](n[74]);
    }
    return {
      key: e,
      first: null,
      c() {
        (o = S("label")),
          (r = S("input")),
          (i = O()),
          (s = j(v)),
          (c = O()),
          (a = S("button")),
          (a.textContent = "?"),
          (u = O()),
          P(r, "type", "checkbox"),
          P(r, "class", "svelte-185rejv"),
          P(a, "class", "svelte-185rejv"),
          P(o, "class", "svelte-185rejv"),
          (this.first = o);
      },
      m(t, e) {
        _(t, o, e),
          b(o, r),
          (r.checked = n[74].active),
          b(o, i),
          b(o, s),
          b(o, c),
          b(o, a),
          b(o, u),
          (h = !0),
          m ||
            ((g = [
              N(r, "change", y),
              N(r, "click", n[17]()),
              N(a, "click", x),
            ]),
            (m = !0));
      },
      p(t, e) {
        (n = t),
          1 & e[0] && (r.checked = n[74].active),
          (!h || 1 & e[0]) &&
            v !==
              (v =
                n[74].path.slice(
                  n[74].path.lastIndexOf("/") + 1,
                  n[74].path.length
                ) + "") &&
            q(s, v);
      },
      r() {
        p = o.getBoundingClientRect();
      },
      f() {
        L(o), $(), J(o, p);
      },
      a() {
        $(), ($ = F(o, p, Nt, {}));
      },
      i(t) {
        h ||
          (Z(() => {
            d && d.end(1), (f = mt(o, n[11], { key: n[74].path })), f.start();
          }),
          (h = !0));
      },
      o(t) {
        f && f.invalidate(), (d = gt(o, n[10], { key: n[74].path })), (h = !1);
      },
      d(t) {
        t && w(o), t && d && d.end(), (m = !1), l(g);
      },
    };
  }
  function ie(t) {
    let e,
      n,
      o,
      r,
      l,
      i,
      s = t[68] + "",
      c = [],
      a = new Map(),
      u = t[0].modules[t[68]].filter($e);
    const f = (t) => t[74].path;
    for (let e = 0; e < u.length; e += 1) {
      let n = Kt(t, u, e),
        o = f(n);
      a.set(o, (c[e] = le(o, n)));
    }
    return {
      c() {
        (e = S("b")), (n = j(s)), (o = S("br")), (r = O());
        for (let t = 0; t < c.length; t += 1) c[t].c();
        l = C();
      },
      m(t, s) {
        _(t, e, s), b(e, n), _(t, o, s), _(t, r, s);
        for (let e = 0; e < c.length; e += 1) c[e].m(t, s);
        _(t, l, s), (i = !0);
      },
      p(t, e) {
        if (
          ((!i || 1 & e[0]) && s !== (s = t[68] + "") && q(n, s), 135169 & e[0])
        ) {
          (u = t[0].modules[t[68]].filter($e)), ut();
          for (let t = 0; t < c.length; t += 1) c[t].r();
          c = $t(c, e, f, 1, t, u, a, l.parentNode, vt, le, l, Kt);
          for (let t = 0; t < c.length; t += 1) c[t].a();
          ft();
        }
      },
      i(t) {
        if (!i) {
          for (let t = 0; t < u.length; t += 1) dt(c[t]);
          i = !0;
        }
      },
      o(t) {
        for (let t = 0; t < c.length; t += 1) pt(c[t]);
        i = !1;
      },
      d(t) {
        t && w(e), t && w(o), t && w(r);
        for (let e = 0; e < c.length; e += 1) c[e].d(t);
        t && w(l);
      },
    };
  }
  function se(e, n) {
    let o,
      r,
      i,
      s,
      c,
      a,
      u,
      f,
      d,
      p,
      h,
      m,
      g,
      v =
        n[74].path.slice(n[74].path.lastIndexOf("/") + 1, n[74].path.length) +
        "",
      $ = t;
    function y() {
      n[40].call(r, n[75], n[76]);
    }
    function x() {
      return n[41](n[74]);
    }
    return {
      key: e,
      first: null,
      c() {
        (o = S("label")),
          (r = S("input")),
          (i = O()),
          (s = j(v)),
          (c = O()),
          (a = S("button")),
          (a.textContent = "?"),
          (u = O()),
          P(r, "type", "checkbox"),
          P(r, "class", "svelte-185rejv"),
          P(a, "class", "svelte-185rejv"),
          P(o, "class", "svelte-185rejv"),
          (this.first = o);
      },
      m(t, e) {
        _(t, o, e),
          b(o, r),
          (r.checked = n[74].active),
          b(o, i),
          b(o, s),
          b(o, c),
          b(o, a),
          b(o, u),
          (h = !0),
          m ||
            ((g = [
              N(r, "change", y),
              N(r, "click", n[17]()),
              N(a, "click", x),
            ]),
            (m = !0));
      },
      p(t, e) {
        (n = t),
          1 & e[0] && (r.checked = n[74].active),
          (!h || 1 & e[0]) &&
            v !==
              (v =
                n[74].path.slice(
                  n[74].path.lastIndexOf("/") + 1,
                  n[74].path.length
                ) + "") &&
            q(s, v);
      },
      r() {
        p = o.getBoundingClientRect();
      },
      f() {
        L(o), $(), J(o, p);
      },
      a() {
        $(), ($ = F(o, p, Nt, {}));
      },
      i(t) {
        h ||
          (Z(() => {
            d && d.end(1), (f = mt(o, n[11], { key: n[74].path })), f.start();
          }),
          (h = !0));
      },
      o(t) {
        f && f.invalidate(), (d = gt(o, n[10], { key: n[74].path })), (h = !1);
      },
      d(t) {
        t && w(o), t && d && d.end(), (m = !1), l(g);
      },
    };
  }
  function ce(t) {
    let e,
      n,
      o,
      r,
      l,
      i,
      s = t[68] + "",
      c = [],
      a = new Map(),
      u = t[0].modules[t[68]].filter(be);
    const f = (t) => t[74].path;
    for (let e = 0; e < u.length; e += 1) {
      let n = Vt(t, u, e),
        o = f(n);
      a.set(o, (c[e] = se(o, n)));
    }
    return {
      c() {
        (e = S("b")), (n = j(s)), (o = S("br")), (r = O());
        for (let t = 0; t < c.length; t += 1) c[t].c();
        l = C();
      },
      m(t, s) {
        _(t, e, s), b(e, n), _(t, o, s), _(t, r, s);
        for (let e = 0; e < c.length; e += 1) c[e].m(t, s);
        _(t, l, s), (i = !0);
      },
      p(t, e) {
        if (
          ((!i || 1 & e[0]) && s !== (s = t[68] + "") && q(n, s), 135169 & e[0])
        ) {
          (u = t[0].modules[t[68]].filter(be)), ut();
          for (let t = 0; t < c.length; t += 1) c[t].r();
          c = $t(c, e, f, 1, t, u, a, l.parentNode, vt, se, l, Vt);
          for (let t = 0; t < c.length; t += 1) c[t].a();
          ft();
        }
      },
      i(t) {
        if (!i) {
          for (let t = 0; t < u.length; t += 1) dt(c[t]);
          i = !0;
        }
      },
      o(t) {
        for (let t = 0; t < c.length; t += 1) pt(c[t]);
        i = !1;
      },
      d(t) {
        t && w(e), t && w(o), t && w(r);
        for (let e = 0; e < c.length; e += 1) c[e].d(t);
        t && w(l);
      },
    };
  }
  function ae(t) {
    let e, n;
    return (
      (e = new Gt({
        props: { $$slots: { default: [ge] }, $$scope: { ctx: t } },
      })),
      e.$on("close", t[42]),
      {
        c() {
          yt(e.$$.fragment);
        },
        m(t, o) {
          xt(e, t, o), (n = !0);
        },
        p(t, n) {
          const o = {};
          (60 & n[0]) | (4194304 & n[2]) && (o.$$scope = { dirty: n, ctx: t }),
            e.$set(o);
        },
        i(t) {
          n || (dt(e.$$.fragment, t), (n = !0));
        },
        o(t) {
          pt(e.$$.fragment, t), (n = !1);
        },
        d(t) {
          _t(e, t);
        },
      }
    );
  }
  function ue(t) {
    let e,
      n,
      o,
      r,
      l,
      i = t[68] + "",
      s = t[69] + "";
    return {
      c() {
        (e = S("p")),
          (n = S("b")),
          (o = j(i)),
          (r = j(" - ")),
          (l = j(s)),
          I(e, "margin-left", "20px");
      },
      m(t, i) {
        _(t, e, i), b(e, n), b(n, o), b(e, r), b(e, l);
      },
      p(t, e) {
        32 & e[0] && i !== (i = t[68] + "") && q(o, i),
          32 & e[0] && s !== (s = t[69] + "") && q(l, s);
      },
      d(t) {
        t && w(e);
      },
    };
  }
  function fe(t) {
    let e,
      n,
      o,
      r,
      l = t[4].funcInfo,
      i = [];
    for (let e = 0; e < l.length; e += 1) i[e] = he(At(t, l, e));
    return {
      c() {
        (e = j("Использование в сценариях: ")), (n = S("br")), (o = O());
        for (let t = 0; t < i.length; t += 1) i[t].c();
        r = C();
      },
      m(t, l) {
        _(t, e, l), _(t, n, l), _(t, o, l);
        for (let e = 0; e < i.length; e += 1) i[e].m(t, l);
        _(t, r, l);
      },
      p(t, e) {
        if (16 & e[0]) {
          let n;
          for (l = t[4].funcInfo, n = 0; n < l.length; n += 1) {
            const o = At(t, l, n);
            i[n]
              ? i[n].p(o, e)
              : ((i[n] = he(o)), i[n].c(), i[n].m(r.parentNode, r));
          }
          for (; n < i.length; n += 1) i[n].d(1);
          i.length = l.length;
        }
      },
      d(t) {
        t && w(e), t && w(n), t && w(o), k(i, t), t && w(r);
      },
    };
  }
  function de(t) {
    let e, n, o;
    return {
      c() {
        (e = S("br")),
          (n = O()),
          (o = S("lable")),
          (o.textContent = "параметры:"),
          I(o, "margin-left", "5px");
      },
      m(t, r) {
        _(t, e, r), _(t, n, r), _(t, o, r);
      },
      d(t) {
        t && w(e), t && w(n), t && w(o);
      },
    };
  }
  function pe(t) {
    let e,
      n,
      o,
      r,
      l,
      i = t[65] + "";
    return {
      c() {
        (e = S("br")),
          (n = O()),
          (o = S("lable")),
          (r = j("- ")),
          (l = j(i)),
          I(o, "margin-left", "40px");
      },
      m(t, i) {
        _(t, e, i), _(t, n, i), _(t, o, i), b(o, r), b(o, l);
      },
      p(t, e) {
        16 & e[0] && i !== (i = t[65] + "") && q(l, i);
      },
      d(t) {
        t && w(e), t && w(n), t && w(o);
      },
    };
  }
  function he(t) {
    let e,
      n,
      o,
      r,
      l,
      i,
      s,
      c,
      a = (t[62].name ? t[62].name : "") + "",
      u = (t[62].descr ? t[62].descr : "") + "",
      f = JSON.stringify(t[62].params).length > 2,
      d = f && de(),
      p = t[62].params,
      h = [];
    for (let e = 0; e < p.length; e += 1) h[e] = pe(Ft(t, p, e));
    return {
      c() {
        (e = S("p")),
          (n = S("b")),
          (o = j(a)),
          (r = j(" - ")),
          (l = j(u)),
          (i = O()),
          d && d.c(),
          (s = O());
        for (let t = 0; t < h.length; t += 1) h[t].c();
        (c = O()), I(e, "margin-left", "20px");
      },
      m(t, a) {
        _(t, e, a),
          b(e, n),
          b(n, o),
          b(e, r),
          b(e, l),
          b(e, i),
          d && d.m(e, null),
          b(e, s);
        for (let t = 0; t < h.length; t += 1) h[t].m(e, null);
        b(e, c);
      },
      p(t, n) {
        if (
          (16 & n[0] &&
            a !== (a = (t[62].name ? t[62].name : "") + "") &&
            q(o, a),
          16 & n[0] &&
            u !== (u = (t[62].descr ? t[62].descr : "") + "") &&
            q(l, u),
          16 & n[0] && (f = JSON.stringify(t[62].params).length > 2),
          f ? d || ((d = de()), d.c(), d.m(e, s)) : d && (d.d(1), (d = null)),
          16 & n[0])
        ) {
          let o;
          for (p = t[62].params, o = 0; o < p.length; o += 1) {
            const r = Ft(t, p, o);
            h[o] ? h[o].p(r, n) : ((h[o] = pe(r)), h[o].c(), h[o].m(e, c));
          }
          for (; o < h.length; o += 1) h[o].d(1);
          h.length = p.length;
        }
      },
      d(t) {
        t && w(e), d && d.d(), k(h, t);
      },
    };
  }
  function me(t) {
    let e, n, o, r;
    return {
      c() {
        (e = S("a")),
          (n = j(
            "Описание модуля в WIKI. Примеры подключения, использования."
          )),
          (r = S("br")),
          P(e, "href", (o = t[4].exampleURL));
      },
      m(t, o) {
        _(t, e, o), b(e, n), _(t, r, o);
      },
      p(t, n) {
        16 & n[0] && o !== (o = t[4].exampleURL) && P(e, "href", o);
      },
      d(t) {
        t && w(e), t && w(r);
      },
    };
  }
  function ge(t) {
    let e,
      n,
      o,
      r,
      l,
      i,
      s,
      c,
      a,
      u,
      f,
      d,
      p,
      h,
      m,
      g,
      v,
      $,
      y,
      x,
      C,
      N,
      M,
      T,
      E,
      R,
      G,
      B,
      U,
      D,
      A,
      F,
      L,
      J,
      V,
      z,
      K,
      H,
      Q,
      W,
      X,
      Y,
      Z,
      tt,
      et,
      nt,
      ot,
      rt,
      lt,
      it,
      st,
      ct,
      at,
      ut = (t[4].title ? t[4].title : "") + "",
      ft = (t[4].moduleName ? t[4].moduleName : "") + "",
      dt = (t[3].name ? t[3].name : "") + "",
      pt = (t[4].moduleDesc ? t[4].moduleDesc : "") + "",
      ht = t[2].menuSection ? "Тип: " : "",
      mt = (t[2].menuSection ? t[2].menuSection : "") + "",
      gt = t[4].moduleVersion ? "Версия: " : "",
      vt = (t[4].moduleVersion ? t[4].moduleVersion : "") + "",
      $t = t[4].propInfo ? "Параметры: " : "",
      bt = (t[4].authorName, ""),
      yt = t[4].authorName ? "Автор: " : "",
      xt = (t[4].authorName ? t[4].authorName : "") + "",
      _t = (t[4].authorContact ? t[4].authorContact : "") + "",
      wt = t[4].authorGit ? "GitHub: " : "",
      kt = (t[4].authorGit ? t[4].authorGit : "") + "",
      St = t[4].specialThanks ? "Благодарности: " : "",
      jt = (t[4].specialThanks ? t[4].specialThanks : "") + "",
      Ot = Object.entries(t[5]),
      Ct = [];
    for (let e = 0; e < Ot.length; e += 1) Ct[e] = ue(Lt(t, Ot, e));
    let Nt = t[4].funcInfo && fe(t),
      Pt = t[4].exampleURL && me(t);
    return {
      c() {
        (e = S("h2")),
          (n = j(ut)),
          (o = O()),
          (r = S("strong")),
          (l = j(ft)),
          (i = j("\n(")),
          (s = j(dt)),
          (c = j(")\n")),
          (a = S("div")),
          (u = j(pt)),
          (f = S("br")),
          (d = O()),
          (p = S("div")),
          (h = j(ht)),
          (m = O()),
          (g = j(mt)),
          (v = O()),
          ($ = j(gt)),
          (y = O()),
          (x = j(vt)),
          (C = S("br")),
          (N = S("br")),
          (M = O()),
          (T = j($t)),
          (E = S("br")),
          (R = O());
        for (let t = 0; t < Ct.length; t += 1) Ct[t].c();
        (G = O()),
          Nt && Nt.c(),
          (B = O()),
          (U = j(bt)),
          (D = S("br")),
          (A = O()),
          Pt && Pt.c(),
          (F = O()),
          (L = j(yt)),
          (J = O()),
          (V = j(xt)),
          (z = S("br")),
          (K = O()),
          (H = S("a")),
          (Q = j(_t)),
          (X = S("br")),
          (Y = O()),
          (Z = j(wt)),
          (tt = O()),
          (et = S("a")),
          (nt = j(kt)),
          (rt = S("br")),
          (lt = O()),
          (it = j(St)),
          (st = O()),
          (ct = j(jt)),
          (at = S("br")),
          P(e, "class", "svelte-185rejv"),
          I(a, "margin-left", "20px"),
          P(H, "href", (W = t[4].authorContact ? t[4].authorContact : "")),
          P(et, "href", (ot = t[4].authorGit ? t[4].authorGit : ""));
      },
      m(t, w) {
        _(t, e, w),
          b(e, n),
          _(t, o, w),
          _(t, r, w),
          b(r, l),
          _(t, i, w),
          _(t, s, w),
          _(t, c, w),
          _(t, a, w),
          b(a, u),
          _(t, f, w),
          _(t, d, w),
          _(t, p, w),
          b(p, h),
          b(p, m),
          b(p, g),
          _(t, v, w),
          _(t, $, w),
          _(t, y, w),
          _(t, x, w),
          _(t, C, w),
          _(t, N, w),
          _(t, M, w),
          _(t, T, w),
          _(t, E, w),
          _(t, R, w);
        for (let e = 0; e < Ct.length; e += 1) Ct[e].m(t, w);
        _(t, G, w),
          Nt && Nt.m(t, w),
          _(t, B, w),
          _(t, U, w),
          _(t, D, w),
          _(t, A, w),
          Pt && Pt.m(t, w),
          _(t, F, w),
          _(t, L, w),
          _(t, J, w),
          _(t, V, w),
          _(t, z, w),
          _(t, K, w),
          _(t, H, w),
          b(H, Q),
          _(t, X, w),
          _(t, Y, w),
          _(t, Z, w),
          _(t, tt, w),
          _(t, et, w),
          b(et, nt),
          _(t, rt, w),
          _(t, lt, w),
          _(t, it, w),
          _(t, st, w),
          _(t, ct, w),
          _(t, at, w);
      },
      p(t, e) {
        if (
          (16 & e[0] &&
            ut !== (ut = (t[4].title ? t[4].title : "") + "") &&
            q(n, ut),
          16 & e[0] &&
            ft !== (ft = (t[4].moduleName ? t[4].moduleName : "") + "") &&
            q(l, ft),
          8 & e[0] &&
            dt !== (dt = (t[3].name ? t[3].name : "") + "") &&
            q(s, dt),
          16 & e[0] &&
            pt !== (pt = (t[4].moduleDesc ? t[4].moduleDesc : "") + "") &&
            q(u, pt),
          4 & e[0] && ht !== (ht = t[2].menuSection ? "Тип: " : "") && q(h, ht),
          4 & e[0] &&
            mt !== (mt = (t[2].menuSection ? t[2].menuSection : "") + "") &&
            q(g, mt),
          16 & e[0] &&
            gt !== (gt = t[4].moduleVersion ? "Версия: " : "") &&
            q($, gt),
          16 & e[0] &&
            vt !== (vt = (t[4].moduleVersion ? t[4].moduleVersion : "") + "") &&
            q(x, vt),
          16 & e[0] &&
            $t !== ($t = t[4].propInfo ? "Параметры: " : "") &&
            q(T, $t),
          32 & e[0])
        ) {
          let n;
          for (Ot = Object.entries(t[5]), n = 0; n < Ot.length; n += 1) {
            const o = Lt(t, Ot, n);
            Ct[n]
              ? Ct[n].p(o, e)
              : ((Ct[n] = ue(o)), Ct[n].c(), Ct[n].m(G.parentNode, G));
          }
          for (; n < Ct.length; n += 1) Ct[n].d(1);
          Ct.length = Ot.length;
        }
        t[4].funcInfo
          ? Nt
            ? Nt.p(t, e)
            : ((Nt = fe(t)), Nt.c(), Nt.m(B.parentNode, B))
          : Nt && (Nt.d(1), (Nt = null)),
          16 & e[0] && bt !== (t[4].authorName, (bt = "")) && q(U, bt),
          t[4].exampleURL
            ? Pt
              ? Pt.p(t, e)
              : ((Pt = me(t)), Pt.c(), Pt.m(F.parentNode, F))
            : Pt && (Pt.d(1), (Pt = null)),
          16 & e[0] &&
            yt !== (yt = t[4].authorName ? "Автор: " : "") &&
            q(L, yt),
          16 & e[0] &&
            xt !== (xt = (t[4].authorName ? t[4].authorName : "") + "") &&
            q(V, xt),
          16 & e[0] &&
            _t !== (_t = (t[4].authorContact ? t[4].authorContact : "") + "") &&
            q(Q, _t),
          16 & e[0] &&
            W !== (W = t[4].authorContact ? t[4].authorContact : "") &&
            P(H, "href", W),
          16 & e[0] &&
            wt !== (wt = t[4].authorGit ? "GitHub: " : "") &&
            q(Z, wt),
          16 & e[0] &&
            kt !== (kt = (t[4].authorGit ? t[4].authorGit : "") + "") &&
            q(nt, kt),
          16 & e[0] &&
            ot !== (ot = t[4].authorGit ? t[4].authorGit : "") &&
            P(et, "href", ot),
          16 & e[0] &&
            St !== (St = t[4].specialThanks ? "Благодарности: " : "") &&
            q(it, St),
          16 & e[0] &&
            jt !== (jt = (t[4].specialThanks ? t[4].specialThanks : "") + "") &&
            q(ct, jt);
      },
      d(t) {
        t && w(e),
          t && w(o),
          t && w(r),
          t && w(i),
          t && w(s),
          t && w(c),
          t && w(a),
          t && w(f),
          t && w(d),
          t && w(p),
          t && w(v),
          t && w($),
          t && w(y),
          t && w(x),
          t && w(C),
          t && w(N),
          t && w(M),
          t && w(T),
          t && w(E),
          t && w(R),
          k(Ct, t),
          t && w(G),
          Nt && Nt.d(t),
          t && w(B),
          t && w(U),
          t && w(D),
          t && w(A),
          Pt && Pt.d(t),
          t && w(F),
          t && w(L),
          t && w(J),
          t && w(V),
          t && w(z),
          t && w(K),
          t && w(H),
          t && w(X),
          t && w(Y),
          t && w(Z),
          t && w(tt),
          t && w(et),
          t && w(rt),
          t && w(lt),
          t && w(it),
          t && w(st),
          t && w(ct),
          t && w(at);
      },
    };
  }
  function ve(t) {
    let e, n, o, r, l, i, s, c, a, u, f, d, p, h;
    n = new Dt({ props: { $$slots: { default: [re] }, $$scope: { ctx: t } } });
    let m = Object.entries(t[0].modules),
      g = [];
    for (let e = 0; e < m.length; e += 1) g[e] = ie(zt(t, m, e));
    const v = (t) =>
      pt(g[t], 1, 1, () => {
        g[t] = null;
      });
    let $ = Object.entries(t[0].modules),
      y = [];
    for (let e = 0; e < $.length; e += 1) y[e] = ce(Jt(t, $, e));
    const x = (t) =>
      pt(y[t], 1, 1, () => {
        y[t] = null;
      });
    let j = t[6] && ae(t);
    return {
      c() {
        (e = S("p")),
          yt(n.$$.fragment),
          (o = O()),
          (r = S("div")),
          (l = S("div")),
          (i = S("h2")),
          (i.textContent = "доступные модули"),
          (s = O());
        for (let t = 0; t < g.length; t += 1) g[t].c();
        (c = O()),
          (a = S("div")),
          (u = S("h2")),
          (u.textContent = "в прошивке"),
          (f = O());
        for (let t = 0; t < y.length; t += 1) y[t].c();
        (d = O()),
          j && j.c(),
          (p = C()),
          P(e, "align", "center"),
          P(i, "class", "svelte-185rejv"),
          P(l, "class", "left svelte-185rejv"),
          P(u, "class", "svelte-185rejv"),
          P(a, "class", "right svelte-185rejv"),
          P(r, "class", "board svelte-185rejv");
      },
      m(t, m) {
        _(t, e, m),
          xt(n, e, null),
          b(e, o),
          _(t, r, m),
          b(r, l),
          b(l, i),
          b(l, s);
        for (let t = 0; t < g.length; t += 1) g[t].m(l, null);
        b(r, c), b(r, a), b(a, u), b(a, f);
        for (let t = 0; t < y.length; t += 1) y[t].m(a, null);
        _(t, d, m), j && j.m(t, m), _(t, p, m), (h = !0);
      },
      p(t, e) {
        const o = {};
        if (
          ((899 & e[0]) | (4194304 & e[2]) &&
            (o.$$scope = { dirty: e, ctx: t }),
          n.$set(o),
          135169 & e[0])
        ) {
          let n;
          for (m = Object.entries(t[0].modules), n = 0; n < m.length; n += 1) {
            const o = zt(t, m, n);
            g[n]
              ? (g[n].p(o, e), dt(g[n], 1))
              : ((g[n] = ie(o)), g[n].c(), dt(g[n], 1), g[n].m(l, null));
          }
          for (ut(), n = m.length; n < g.length; n += 1) v(n);
          ft();
        }
        if (135169 & e[0]) {
          let n;
          for ($ = Object.entries(t[0].modules), n = 0; n < $.length; n += 1) {
            const o = Jt(t, $, n);
            y[n]
              ? (y[n].p(o, e), dt(y[n], 1))
              : ((y[n] = ce(o)), y[n].c(), dt(y[n], 1), y[n].m(a, null));
          }
          for (ut(), n = $.length; n < y.length; n += 1) x(n);
          ft();
        }
        t[6]
          ? j
            ? (j.p(t, e), 64 & e[0] && dt(j, 1))
            : ((j = ae(t)), j.c(), dt(j, 1), j.m(p.parentNode, p))
          : j &&
            (ut(),
            pt(j, 1, 1, () => {
              j = null;
            }),
            ft());
      },
      i(t) {
        if (!h) {
          dt(n.$$.fragment, t);
          for (let t = 0; t < m.length; t += 1) dt(g[t]);
          for (let t = 0; t < $.length; t += 1) dt(y[t]);
          dt(j), (h = !0);
        }
      },
      o(t) {
        pt(n.$$.fragment, t), (g = g.filter(Boolean));
        for (let t = 0; t < g.length; t += 1) pt(g[t]);
        y = y.filter(Boolean);
        for (let t = 0; t < y.length; t += 1) pt(y[t]);
        pt(j), (h = !1);
      },
      d(t) {
        t && w(e),
          _t(n),
          t && w(r),
          k(g, t),
          k(y, t),
          t && w(d),
          j && j.d(t),
          t && w(p);
      },
    };
  }
  const $e = (t) => !t.active,
    be = (t) => t.active;
  function ye(t, e, n) {
    let o =
        "https://raw.githubusercontent.com/IoTManagerProject/IoTManager/ver4dev/myProfile.json?1",
      r = {},
      l = 0,
      i = {},
      s = [],
      c = [],
      a = [],
      u = [],
      f = !1,
      d = "dev",
      p = {};
    async function h(t) {
      try {
        let e = await fetch(t, { mode: "cors", method: "GET" });
        e.ok ? n(0, (r = await e.json())) : console.log("error", e.statusText);
      } catch (t) {
        console.log(t);
      }
    }
    (r = {
      iotmSettings: {},
      projectProp: { platformio: {} },
      modules: {
        "Виртуальные элементы": [],
        Сенсоры: [],
        executive_devices: [],
        Экраны: [],
      },
    }),
      h(o),
      (async function (t) {
        try {
          let e = await fetch(t, { mode: "cors", method: "GET" });
          e.ok ? (p = await e.json()) : console.log("error", e.statusText);
        } catch (t) {
          console.log(t);
        }
      })("https://iotmanager.org/firmwarebuilder/moduleSize.json?1");
    const [m, g] = Ct({
      fallback(t, e) {
        const n = getComputedStyle(t),
          o = "none" === n.transform ? "" : n.transform;
        return {
          duration: 600,
          easing: Ot,
          css: (t) =>
            `\n\t\t\t\t\ttransform: ${o} scale(${t});\n\t\t\t\t\topacity: ${t}\n\t\t\t\t`,
        };
      },
    });
    function v(t) {
      console.log(t.path),
        (o =
          "https://raw.githubusercontent.com/IoTManagerProject/IoTManager/ver4stable/" +
          JSON.parse(JSON.stringify(t.path).replace(/\\\\/g, "/")) +
          "/modinfo.json"),
        (i = []),
        n(2, (s = [])),
        n(3, (c = [])),
        n(3, (c.name = "Данные загружаются..."), c),
        n(4, (a = [])),
        n(5, (u = [])),
        (async function (t) {
          i = [];
          try {
            let e = await fetch(t, { mode: "cors", method: "GET" });
            e.ok
              ? ((i = await e.json()),
                n(2, (s = i)),
                n(3, (c = i.configItem[0])),
                n(4, (a = i.about)),
                n(5, (u = a.propInfo)))
              : console.log("error", e.statusText);
          } catch (t) {
            console.log(t);
          }
        })(o),
        n(6, (f = !0));
    }
    let $ = !0;
    function b(t) {
      n(8, ($ = t));
    }
    function y() {
      (o =
        "dev" == d
          ? "https://raw.githubusercontent.com/IoTManagerProject/IoTManager/ver4dev/myProfile.json?1"
          : "https://raw.githubusercontent.com/IoTManagerProject/IoTManager/ver4stable/myProfile.json?1"),
        h(o);
    }
    function x() {
      n(1, (l = 0)),
        n(0, (r.projectProp.platformio.default_envs = _.text), r),
        k();
    }
    let _ = [],
      w = [
        { id: 1, text: "esp8266_4mb" },
        { id: 2, text: "esp32_4mb" },
        { id: 3, text: "esp8266_1mb" },
        { id: 4, text: "esp8266_1mb_ota" },
      ];
    function k() {
      n(1, (l = 0));
      for (const [t, e] of Object.entries(r.modules))
        for (const [e, o] of Object.entries(
          r.modules[t].filter((t) => t.active)
        )) {
          let t = o.path.slice(o.path.lastIndexOf("/") + 1, o.path.length);
          p[t] && n(1, (l += Math.round(p[t][_.text])));
        }
      "esp8266_4mb" == _.text && n(1, (l = Math.round(l / 6110))),
        "esp32_4mb" == _.text && n(1, (l = Math.round(l / 3243))),
        "esp8266_1mb" == _.text && n(1, (l = Math.round(l / 3237))),
        "esp8266_1mb_ota" == _.text && n(1, (l = Math.round(l / 2601)));
    }
    function S() {
      var t = document.createElement("a");
      (t.href = window.URL.createObjectURL(
        new Blob([JSON.stringify(r)], { type: "text/plain" })
      )),
        (t.download = "myProfile.json"),
        console.log(JSON.stringify(r));
    }
    return [
      r,
      l,
      s,
      c,
      a,
      u,
      f,
      d,
      $,
      _,
      m,
      g,
      v,
      b,
      y,
      x,
      w,
      k,
      S,
      function (t) {
        (l = t), n(1, l);
      },
      function (t) {
        (l = t), n(1, l);
      },
      function (t) {
        (l = t), n(1, l);
      },
      function (t) {
        (l = t), n(1, l);
      },
      function () {
        (d = E(this)), n(7, d);
      },
      () => y(),
      function () {
        (_ = E(this)), n(9, _), n(16, w);
      },
      () => x(),
      () => b(!0),
      () => b(!1),
      function () {
        (r.iotmSettings.routerssid = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.routerpass = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.mqttServer = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.mqttPort = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.mqttPrefix = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.mqttUser = this.value), n(0, r);
      },
      function () {
        (r.iotmSettings.mqttPass = this.value), n(0, r);
      },
      function () {
        (r = this.value), n(0, r);
      },
      () => S(),
      function (t, e) {
        (t[e].active = this.checked), n(0, r);
      },
      (t) => v(t),
      function (t, e) {
        (t[e].active = this.checked), n(0, r);
      },
      (t) => v(t),
      () => n(6, (f = !1)),
    ];
  }
  return new (class extends St {
    constructor(t) {
      super(), kt(this, t, ye, ve, s, {}, null, [-1, -1, -1]);
    }
  })({ target: document.body });
})();
//# sourceMappingURL=bundle.js.map
