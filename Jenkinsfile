pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      steps {
        echo "Building..."
      }
      steps {
        hostname
      }
      steps {
        /usr/bin/git --version
      }
      steps {
        /usr/bin/gcc --version
      }
      steps {
        /usr/bin/g++ --version
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
