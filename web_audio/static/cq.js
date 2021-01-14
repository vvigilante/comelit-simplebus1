export default class CircularQueue {
    constructor(size) {
        this.element = [];
        this.size = size
        this.length = 0
        this.front = 0
        this.back = -1
    }
    isEmpty() {
        return (this.length == 0)
    }
    enqueue(element) {
        if (this.length >= this.size) {
            //throw (new Error("Maximum length exceeded"))
        } else {
            this.back = (this.back + 1) % this.size
            this.element[this.back] = element
            this.length++
        }
    }
    dequeue() {
        if (this.isEmpty()) throw (new Error("No elements in the queue"))
        const value = this.getFront()
        this.element[this.front] = null
        this.front = (this.front + 1) % this.size
        this.length--
        return value
    }
    getFront() {
        if (this.isEmpty()) throw (new Error("No elements in the queue"))
        return this.element[this.front]
    }
    clear() {
        this.element = new Array()
        this.length = 0
        this.back = 0
        this.front = -1
    }
}