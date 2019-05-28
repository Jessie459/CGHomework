## Vue.js代码规范
### 必要的
#### 组件名为多个单词
组件名应该始终为多个单词，根组件`App`以及`<transition>`、`<component>`等Vue内置组件除外。这样做可以避免和现有的以及未来的HTML元素发生冲突，因为所有的HTML元素名称都是单个单词。  
> 示例：  
```javascript
Vue.component('todo-item', {
  // ...
})
export default {
  name: 'TodoItem',
  // ...
}
```